#include "json_manager.hpp"

#include <iostream>

JsonManager& JsonManager::addParam(const std::string& _Name, bool _v)
{
    DBLK(this->checkCreate();)

    Writer_.Key(_Name.c_str());
    Writer_.Bool(_v);

    return *this;
}

JsonManager& JsonManager::addParam(const std::string& _Name, double _v)
{
    DBLK(this->checkCreate();)

    Writer_.Key(_Name.c_str());
    Writer_.Double(_v);

    return *this;
}

JsonManager& JsonManager::addParam(const std::string& _Name, std::uint32_t _v)
{
    DBLK(this->checkCreate();)

    Writer_.Key(_Name.c_str());
    Writer_.Uint(_v);

    return *this;
}

JsonManager& JsonManager::addParam(const std::string& _Name, const std::string& _v)
{
    DBLK(this->checkCreate();)

    Writer_.Key(_Name.c_str());
    Writer_.String(_v.c_str());

    return *this;
}

JsonManager& JsonManager::addParam(const std::string& _Name, const char* _v)
{
    DBLK(this->checkCreate();)

    Writer_.Key(_Name.c_str());
    Writer_.String(_v);

    return *this;
}

JsonManager& JsonManager::beginObject()
{
    Writer_.StartObject();
    return *this;
}

JsonManager& JsonManager::endObject()
{
    Writer_.EndObject();
    return *this;
}

JsonManager& JsonManager::createNotification(const std::string& _Notification)
{
    MessageType_ = MessageType::NOTIFICATION;
    this->createHeaderNotificationRequest(_Notification);
    return *this;
}

JsonManager& JsonManager::createRequest(const std::string& _Req)
{
    MessageType_ = MessageType::REQUEST;
    this->createHeaderNotificationRequest(_Req);
    return *this;
}

JsonManager& JsonManager::createResult()
{
    MessageType_ = MessageType::RESULT;
    this->createHeaderResult();
    return *this;
}

JsonManager& JsonManager::createResult(bool _r)
{
    this->createHeaderResult();
    Writer_.Bool(_r);
    return *this;
}

JsonManager& JsonManager::createResult(const char* _r)
{
    this->createHeaderResult();
    Writer_.String(_r);
    return *this;
}

JsonManager& JsonManager::createResult(const std::string& _r)
{
    this->createHeaderResult();
    Writer_.String(_r.c_str());
    return *this;
}

JsonManager::RequestIDType JsonManager::send(ClientIDType _ClientID, RequestIDType _ReqID)
{
    DBLK(
        auto& Messages = Reg_.ctx<MessageHandler>();
        if (!IsMessageCreated_)
            Messages.report("jsn", "No message created", MessageHandler::ERROR);

        IsMessageCreated_ = false;
        IsMessageSend_ = true;
    )

    if (MessageType_ == MessageType::REQUEST || MessageType_ == MessageType::NOTIFICATION)
    {
        Writer_.EndObject();
        Writer_.Key("id");
        Writer_.Uint(++RequestID_);
    }
    else if (MessageType_ == MessageType::RESULT || MessageType_ == MessageType::ERROR)
    {
        Writer_.Key("id");
        Writer_.Uint(_ReqID);
    }
    Writer_.EndObject();
    OutputQueue_->enqueue(NetworkMessage{_ClientID, Buffer_.GetString()});

    Buffer_.Clear();
    Writer_.Reset(Buffer_);

    return RequestID_;
}

void JsonManager::createHeaderJsonRcp()
{
    DBLK(
        auto& Messages = Reg_.ctx<MessageHandler>();
        if (!IsMessageSend_)
        {
            Messages.report("jsn", "Previous message was constructed but not send", MessageHandler::WARNING);
            Buffer_.Clear();
            Writer_.Reset(Buffer_);
        }

        IsMessageCreated_ = true;
        IsMessageSend_ = false;
    )

    Writer_.StartObject();
    Writer_.Key("jsonrpc"); Writer_.String("2.0");
}

void JsonManager::createHeaderNotificationRequest(const std::string& _m)
{
    this->createHeaderJsonRcp();

    Writer_.Key("method"); Writer_.String(_m.c_str());
    Writer_.Key("params");
    Writer_.StartObject();
}

void JsonManager::createHeaderResult()
{
    MessageType_ = MessageType::RESULT;
    this->createHeaderJsonRcp();
    Writer_.Key("result");
}

DBLK(
    void JsonManager::checkCreate()
    {
        auto& Messages = Reg_.ctx<MessageHandler>();
        if (!IsMessageCreated_)
            Messages.report("jsn", "No message created", MessageHandler::ERROR);
    }
)
