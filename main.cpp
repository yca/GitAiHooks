#include "openai.h"

#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCoreApplication>

const auto defaultSys = R"(
You will be provided with a commit diff, you're required to write a suitable commit message for this diff. Remember that each line should not be longer than 80 characters. Please format your commit message in the following format:

```
One line description of the mesage.

Followed by details, after leaving an empty line. If summary is in bullets, you should use '*' for the bullet items.
```

Once again, don't exceed 80 character limit per line please.
)";

static QString runProcess(const QStringList &args)
{
	QProcess p;
	p.setProgram("git");
	p.setArguments(args);
	p.start();
	p.waitForFinished();
	return QString::fromUtf8(p.readAllStandardOutput());
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QCoreApplication::setOrganizationName("Sparse");
	QCoreApplication::setOrganizationDomain("sparsetechnology.com");
	QCoreApplication::setApplicationName("GitAiHooks");

	QSettings sets;

	auto args = a.arguments();

	if (args.contains("--set-api-key")) {
		sets.setValue("openai_key", args[args.indexOf("--set-api-key") + 1]);
		return 0;
	}
	auto key = sets.value("openai_key").toString();
	if (key.isEmpty()) {
		fprintf(stdout, "please enter your API key if you want to use Git AI hooks:\n");
		fprintf(stdout, "	GitAiHooks --set-api-key \"sk-...\"\n");
		return -3;
	}

	QString commitTempFile;
	auto ind = args.indexOf("--pwd");
	if (ind >= 0 && ind + 1 < args.size())
		QDir::setCurrent(args[ind + 1]);
	else if (args.size() == 2) {
		/* normal commit */
		commitTempFile = args[1];
	} else {
		/* ammend or merge commit */
		a.exit(0);
	}

	OpenAI gpt(key);
	gpt.useModel("gpt-4-1106-preview");
	QObject::connect(&gpt, &OpenAI::encodingResultsReady, &a, [&](const QJsonDocument& jsonDoc, const QVariant &var) {
		const auto &choice = jsonDoc["choices"].toArray().first().toObject();
		auto fres = choice["finish_reason"].toString();
		if (fres != "stop") {
			fprintf(stderr, "un-expected finish reason '%s'", fres.toStdString().data());
			a.exit(-1);
		}

		auto resp = choice["message"].toObject()["content"].toString();
		if (commitTempFile.isEmpty()) {
			fprintf(stdout, "%s", resp.toStdString().data());
			fprintf(stdout, "\n");
			fprintf(stdout, "%s\n", a.arguments().join(' ').toStdString().data());
		} else {
			QFile f(commitTempFile);
			f.open(QIODevice::WriteOnly);
			f.write(resp.toUtf8());
			f.write("\n");
			f.close();
		}

		a.exit(0);
	});
	QObject::connect(&gpt, &OpenAI::encodingApiError, &a, [&](const QString &errmes, const QVariant &var) {
		fprintf(stderr, "error '%s' during API call", errmes.toStdString().data());
		a.exit(-2);
	});

	QStringList outlines;
	const auto &lines = runProcess(QStringList() << "diff" << "--cached").split('\n');
	for (const auto &line: lines)
		outlines << line;

	QString system = defaultSys;
	gpt.chat(system, outlines.join('\n'), "default");

	return a.exec();
}
