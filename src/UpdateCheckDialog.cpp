#include "UpdateCheckDialog.h"
#include "ui_UpdateCheckDialog.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include "Utils.h"

UpdateCheckDialog::UpdateCheckDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::UpdateCheckDialog)
{
	ui->setupUi(this);

	ui->lblCurrentVersion->setText(tr("Current Version: %0").arg(QCoreApplication::applicationVersion()));
	ui->lblLatestVersion->setText(tr("Latest Version: %0").arg(tr("Checking...")));

	connect(
	 &networkAccess, SIGNAL (finished(QNetworkReply*)),
	 this, SLOT (fileDownloaded(QNetworkReply*))
	 );

	QNetworkRequest request(QUrl("https://fuel-scm.org/fossil/timeline.rss"));
	networkAccess.get(request);

	//QString banner(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
}

UpdateCheckDialog::~UpdateCheckDialog()
{
	delete ui;
}

struct Version
{
	uint Major;
	uint Minor;
	uint Build;

	Version(const QString &version)
	{
		QStringList sections = version.split(".");
		TrimStringList(sections);
		Q_ASSERT(sections.size()==3);
		bool ok = false;
		Major = sections[0].toUInt(&ok);
		Q_ASSERT(ok);
		Minor = sections[1].toUInt(&ok);
		Q_ASSERT(ok);
		Build = sections[2].toUInt(&ok);
		Q_ASSERT(ok);
	}

	bool operator < (const Version &other) const
	{
		if(Major < other.Major)
			return true;
		else if (Major > other.Major)
			return false;

		if(Minor < other.Minor)
			return true;
		else if (Minor > other.Minor)
			return false;

		if(Build < other.Build)
			return true;
		return false;
	}

	bool operator == (const Version &other) const
	{
		return Major == other.Major && Minor == other.Minor && Build == other.Build;
	}
};


//-----------------------------------------------------------------------------
void UpdateCheckDialog::fileDownloaded(QNetworkReply *reply)
{
	if(!reply)
	{
		ui->lblLatestVersion->setText(tr("Could not connect to server."));
		return;
	}

	reply->deleteLater();

	if(reply && reply->error() != QNetworkReply::NoError)
	{
		ui->lblLatestVersion->setText(tr("Error: %0").arg(reply->errorString()));
		return;
	}

	//QByteArray data = reply->readAll();
	QFile f("c:\\temp\\versions.txt");
	Q_ASSERT(f.open(QFile::ReadOnly));
	QByteArray data = f.readAll();
	f.close();

	QStringMap props;
	ParseProperties(props, QString(data).split('\n'), '=');
	const QString lastest_key = QCoreApplication::applicationName() + ".Latest";

	if(props.find(lastest_key)==props.end())
	{
		ui->lblLatestVersion->setText(tr("Error: %0").arg(tr("Invalid format")));
		return;
	}

	Version vcurr(QCoreApplication::applicationVersion());
	Version vlatest(props[lastest_key]);

	ui->lblLatestVersion->setText(QString(data));
	ui->lblLatestVersion->setText(tr("Latest Version: %0").arg(props[lastest_key]));
}
