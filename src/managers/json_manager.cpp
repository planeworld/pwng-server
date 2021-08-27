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

JsonManager& JsonManager::addParam(const std::string& _Name, std::uint64_t _v)
{
    DBLK(this->checkCreate();)

    Writer_.Key(_Name.c_str());
    Writer_.Uint64(_v);

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

JsonManager& JsonManager::addValue(double _v)
{
    DBLK(this->checkCreate();)

    Writer_.Double(_v);

    return *this;
}

JsonManager& JsonManager::addValue(std::uint32_t _v)
{
    DBLK(this->checkCreate();)

    Writer_.Uint(_v);

    return *this;
}

JsonManager& JsonManager::beginArray()
{
    Writer_.StartArray();
    return *this;
}

JsonManager& JsonManager::beginArray(const char* _Key)
{
    Writer_.Key(_Key);
    Writer_.StartArray();
    return *this;
}

JsonManager& JsonManager::endArray()
{
    Writer_.EndArray();
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

JsonManager& JsonManager::createError()
{
    this->createHeaderError();
    Writer_.StartObject();
    Writer_.Key("code"); Writer_.Int(-32601);
    Writer_.Key("message"); Writer_.String("Method not found");
    Writer_.EndObject();
    return *this;
}

JsonManager& JsonManager::createResult()
{
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

void JsonManager::finalise(RequestIDType _ReqID)
{
    DBLK(this->checkCreate();
        IsMessageCreated_ = false;
        IsMessageFinalised_ = true;
    )
    if (MessageType_ == MessageType::REQUEST || MessageType_ == MessageType::NOTIFICATION)
    {
        Writer_.EndObject();
        if (MessageType_ == MessageType::REQUEST)
        {
            Writer_.Key("id");
            Writer_.Uint(++RequestID_);
        }
    }
    else // if (MessageType_ == MessageType::RESULT || MessageType_ == MessageType::ERROR)
    {
        Writer_.Key("id");
        Writer_.Uint(_ReqID);
    }
    Writer_.EndObject();
}

void JsonManager::createHeaderJsonRcp()
{
    Buffer_.Clear();
    Writer_.Reset(Buffer_);

    DBLK(
        auto& Messages = Reg_.ctx<MessageHandler>();
        if (!IsMessageFinalised_)
        {
            Messages.report("jsn", "Previous message was constructed but not finalised", MessageHandler::WARNING);
        }

        IsMessageCreated_ = true;
        IsMessageFinalised_ = false;
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

void JsonManager::createHeaderError()
{
    MessageType_ = MessageType::ERROR;
    this->createHeaderJsonRcp();
    Writer_.Key("error");
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
