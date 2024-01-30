#include "openai.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

OpenAI::OpenAI(const QString &apiKey, QObject *parent)
	: QObject{parent},
	  m_apiKey(apiKey)
{
	modelDesc = "gpt-3.5-turbo-1106";
	connect(&m_networkManager, &QNetworkAccessManager::finished,
			this, &OpenAI::onRequestFinished);
}

void OpenAI::encode(const QString &text, const QVariant &var)
{
	QNetworkRequest request(QUrl("https://api.openai.com/v1/embeddings"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

	QJsonObject jsonObject;
	jsonObject["input"] = text;
	jsonObject["model"] = "text-embedding-ada-002";

	QJsonDocument jsonDoc(jsonObject);
	QByteArray jsonData = jsonDoc.toJson();

	auto *reply = m_networkManager.post(request, jsonData);
	requestMap[reply] = var;
}

void OpenAI::chat(const QString &system, const QString &user, const QVariant &var)
{
	QNetworkRequest request(QUrl("https://api.openai.com/v1/chat/completions"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

	QJsonObject jsonObject;
	jsonObject["model"] = modelDesc;
	QJsonArray messages;
	{
		QJsonObject obj;
		obj["role"] = "system";
		obj["content"] = system;
		messages << obj;
	}
	{
		QJsonObject obj;
		obj["role"] = "user";
		obj["content"] = user;
		messages << obj;
	}
	jsonObject["messages"] = messages;
	if (0) {
		QJsonObject obj;
		obj["type"] = "json_object";
		jsonObject["response_format"] = obj;
	}

	QJsonDocument jsonDoc(jsonObject);
	QByteArray jsonData = jsonDoc.toJson();

	auto *reply = m_networkManager.post(request, jsonData);
	requestMap[reply] = var;
}

void OpenAI::onRequestFinished(QNetworkReply *reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QByteArray responseData = reply->readAll();

		QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
		emit encodingResultsReady(jsonDoc, requestMap[reply]);
	} else {
		qWarning() << "API request error: " << reply->errorString();
		emit encodingApiError(reply->errorString(), requestMap[reply]);
	}
	requestMap.remove(reply);

	reply->deleteLater();
}

void OpenAI::useModel(const QString &modelName)
{
	modelDesc = modelName;
}

float OpenAI::getPricing()
{
	if (modelDesc == "gpt-3.5-turbo-1106")
		return 0.0010;
	if (modelDesc == "gpt-4-1106-preview")
		return 0.010;
	return 0;
}
