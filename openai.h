#ifndef OPENAI_H
#define OPENAI_H

#include <QHash>
#include <QObject>
#include <QVariant>
#include <QNetworkAccessManager>

class QNetworkReply;

class OpenAI : public QObject
{
	Q_OBJECT
public:
	explicit OpenAI(const QString& apiKey, QObject *parent = nullptr);

	void useModel(const QString &modelName);
	float getPricing();

	void encode(const QString& text, const QVariant &var = QVariant());
	void chat(const QString &system, const QString &user, const QVariant &var);

signals:
	void encodingResultsReady(const QJsonDocument& jsonDoc, const QVariant &var);
	void encodingApiError(const QString &errmes, const QVariant &var);

private slots:
	void onRequestFinished(QNetworkReply* reply);

private:
	QString m_apiKey;
	QString modelDesc;
	QNetworkAccessManager m_networkManager;
	QHash<QNetworkReply *, QVariant> requestMap;

};

#endif // OPENAI_H
