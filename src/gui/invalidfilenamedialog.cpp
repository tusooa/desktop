#include "invalidfilenamedialog.h"
#include "accountfwd.h"
#include "common/syncjournalfilerecord.h"
#include "propagateremotemove.h"
#include "ui_invalidfilenamedialog.h"

#include <folder.h>

#include <QPushButton>
#include <QDir>
#include <qabstractbutton.h>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QPushButton>
#include <QRegExp>

namespace {
const QRegExp invalidCharactersRegex("[\\/:?*\"<>|]");
}

namespace OCC {

InvalidFilenameDialog::InvalidFilenameDialog(AccountPtr account, Folder *folder, QString filePath, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::InvalidFilenameDialog)
    , _account(account)
    , _folder(folder)
    , _filePath(std::move(filePath))
{
    Q_ASSERT(_account);
    Q_ASSERT(_folder);

    const auto filePathFileInfo = QFileInfo(_filePath);
    _relativeFilePath = filePathFileInfo.path() + QStringLiteral("/");
    _relativeFilePath = _relativeFilePath.replace(folder->path(), QStringLiteral(""));
    _relativeFilePath = _relativeFilePath.isEmpty() ? QStringLiteral("") : _relativeFilePath + QStringLiteral("/");

    _originalFileName = _relativeFilePath + filePathFileInfo.fileName();

    _ui->setupUi(this);
    _ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    _ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Rename file"));

    _ui->descriptionLabel->setText(tr("The file %1 could not be synced because it contains characters which are not allowed on this system.").arg(_originalFileName));
    _ui->filenameLineEdit->setText(filePathFileInfo.fileName());

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(_ui->filenameLineEdit, &QLineEdit::textChanged, this,
        &InvalidFilenameDialog::onFilenameLineEditTextChanged);
}

InvalidFilenameDialog::~InvalidFilenameDialog() = default;

void InvalidFilenameDialog::accept()
{
    _newFilename = _relativeFilePath + _ui->filenameLineEdit->text().trimmed();
    const auto propfindJob = new PropfindJob(_account, QDir::cleanPath(_folder->remotePath() + _newFilename));
    connect(propfindJob, &PropfindJob::result, this, &InvalidFilenameDialog::onPropfindSuccess);
    connect(propfindJob, &PropfindJob::finishedWithError, this, &InvalidFilenameDialog::onPropfindError);
    propfindJob->start();
}

void InvalidFilenameDialog::onFilenameLineEditTextChanged(const QString &text)
{
    const auto isNewFileNameDifferent = text != _originalFileName;
    const auto containsIllegalChars = text.contains(invalidCharactersRegex) || text.endsWith(QLatin1Char('.'));
    const auto isTextValid = isNewFileNameDifferent && !containsIllegalChars;

    if (isTextValid) {
        _ui->errorLabel->setText("");
    } else {
        _ui->errorLabel->setText("Filename contains illegal characters.");
    }

    _ui->buttonBox->button(QDialogButtonBox::Ok)
        ->setEnabled(isTextValid);
}

void InvalidFilenameDialog::onMoveJobFinished()
{
    const auto job = qobject_cast<MoveJob *>(sender());
    const auto error = job->reply()->error();

    if (error != QNetworkReply::NoError) {
        _ui->errorLabel->setText("Could not rename file. Please make sure you are connected to the server.");
        return;
    }

    QDialog::accept();
}

void InvalidFilenameDialog::onPropfindSuccess(const QVariantMap &values)
{
    Q_UNUSED(values);

    // File does exist. Can not rename.
    _ui->errorLabel->setText(tr("Can not rename file because file with the same name does already exist on the server. Please pick another name."));
    _ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void InvalidFilenameDialog::onPropfindError(QNetworkReply *reply)
{
    Q_UNUSED(reply);

    // File does not exist. We can rename it.
    const auto remoteSource = QDir::cleanPath(_folder->remotePath() + _originalFileName);
    const auto remoteDestionation = QDir::cleanPath(_account->davUrl().path() + _folder->remotePath() + _newFilename);
    const auto moveJob = new MoveJob(_account, remoteSource, remoteDestionation, this);
    connect(moveJob, &MoveJob::finishedSignal, this, &InvalidFilenameDialog::onMoveJobFinished);
    moveJob->start();
}

void InvalidFilenameDialog::setExplanation(const QString &message)
{
    _ui->explanationLabel->setText(message);
}
}
