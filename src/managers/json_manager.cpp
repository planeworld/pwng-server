#include "json_manager.hpp"

#include <iostream>

#include <rapidjson/document.h>

JsonManager& JsonManager::addParam(const std::string& _Name, bool _v)
{
    DBLK(this->checkCreate();)

    this->checkParamBegin();
    Writer_.Key(_Name.c_str());
    Writer_.Bool(_v);

    return *this;
}

JsonManager& JsonManager::addParam(const std::string& _Name, double _v)
{
    DBLK(this->checkCreate();)

    this->checkParamBegin();
    Writer_.Key(_Name.c_str());
    Writer_.Double(_v);

    return *this;
}

JsonManager& JsonManager::addParam(const std::string& _Name, std::uint32_t _v)
{
    DBLK(this->checkCreate();)

    this->checkParamBegin();
    Writer_.Key(_Name.c_str());
    Writer_.Uint(_v);

    return *this;
}

JsonManager& JsonManager::addParam(const std::string& _Name, std::uint64_t _v)
{
    DBLK(this->checkCreate();)

    this->checkParamBegin();
    Writer_.Key(_Name.c_str());
    Writer_.Uint64(_v);

    return *this;
}

JsonManager& JsonManager::addParam(const std::string& _Name, const std::string& _v)
{
    DBLK(this->checkCreate();)

    this->checkParamBegin();
    Writer_.Key(_Name.c_str());
    Writer_.String(_v.c_str());

    return *this;
}

JsonManager& JsonManager::addParam(const std::string& _Name, const char* _v)
{
    DBLK(this->checkCreate();)

    this->checkParamBegin();
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

JsonManager& JsonManager::addValue(int _v)
{
    DBLK(this->checkCreate();)

    Writer_.Int(_v);

    return *this;
}

JsonManager& JsonManager::addValue(const char* _v)
{
    DBLK(this->checkCreate();)

    Writer_.String(_v);

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

JsonManager& JsonManager::createError(ErrorType _e)
{
    this->createHeaderError();
    Writer_.StartObject();
    switch (_e)
    {
        case ErrorType::METHOD:
            Writer_.Key("code"); Writer_.Int(-32601);
            Writer_.Key("message"); Writer_.String("Method not found");
            break;
        case ErrorType::PARAMS:
            Writer_.Key("code"); Writer_.Int(-32602);
            Writer_.Key("message"); Writer_.String("Invalid params");
            break;
        case ErrorType::PARSE:
            Writer_.Key("code"); Writer_.Int(-32700);
            Writer_.Key("message"); Writer_.String("Parse error");
            break;
        case ErrorType::REQUEST:
            Writer_.Key("code"); Writer_.Int(-32600);
            Writer_.Key("message"); Writer_.String("Invalid Request");
            break;
    }
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

JsonManager::ParamCheckResult JsonManager::checkParams(const NetworkDocument& _d, std::vector<ParamsType> _p)
{
    ParamCheckResult ReturnValue;

    auto& Messages = Reg_.ctx<MessageHandler>();

    if (_d.Payload->HasMember("params"))
    {
        const auto& ParamsArray = (*_d.Payload)["params"].GetArray();
        if (ParamsArray.Size() == _p.size())
        {
            for (auto i=0u; i < ParamsArray.Size(); ++i)
            {
                bool ParamTypeOkay{false};
                switch (_p[0])
                {
                    case ParamsType::NUMBER:
                        if (ParamsArray[0].IsNumber())
                        {
                            ReturnValue = {true, ErrorType::PARAMS, _d.ClientID, 0};
                            ParamTypeOkay = true;
                        }
                        break;
                    case ParamsType::STRING:
                        if (ParamsArray[0].IsString())
                        {
                            ReturnValue = {true, ErrorType::PARAMS, _d.ClientID, 0};
                            ParamTypeOkay = true;
                        }
                        break;
                }
                if (!ParamTypeOkay)
                {
                    Messages.report("jsn", "Wrong parameter types.", MessageHandler::WARNING);
                    if (_d.Payload->HasMember("id"))
                    {
                        ReturnValue = {false, ErrorType::PARAMS, _d.ClientID, (*_d.Payload)["id"].GetUint()};
                    }
                    else
                    {
                        Messages.report("jsn", "Invalid request object, no id send", MessageHandler::WARNING);
                        ReturnValue = {false, ErrorType::PARAMS, _d.ClientID, 0};
                        ReturnValue = {false, ErrorType::REQUEST, _d.ClientID, 0};
                    }
                }
            }
        }
        else
        {
            Messages.report("jsn", "Wrong number of parameters: "+
                            std::to_string(ParamsArray.Size())+" of "+
                            std::to_string(_p.size()), MessageHandler::WARNING);
            if (_d.Payload->HasMember("id"))
            {
                ReturnValue = {false, ErrorType::PARAMS, _d.ClientID, (*_d.Payload)["id"].GetUint()};
            }
            else
            {
                Messages.report("jsn", "Invalid request object, no id send", MessageHandler::WARNING);
                ReturnValue = {false, ErrorType::PARAMS, _d.ClientID, 0};
                ReturnValue = {false, ErrorType::REQUEST, _d.ClientID, 0};
            }
        }
    }
    else
    {
        bool r = true;
        if (_p.size() > 0)
        {
            r = false;
            Messages.report("jsn", "Parameter member missing: 0 of " + std::to_string(_p.size()) + " parameters.", MessageHandler::WARNING);
        }
        if (_d.Payload->HasMember("id"))
        {
            ReturnValue = {r, ErrorType::PARAMS, _d.ClientID, (*_d.Payload)["id"].GetUint()};
        }
        else
        {
            Messages.report("jsn", "Invalid request object, no id send", MessageHandler::WARNING);
            ReturnValue = {false, ErrorType::REQUEST, _d.ClientID, 0};
        }
    }

    return ReturnValue;
}

void JsonManager::finalise(RequestIDType _ReqID)
{
    DBLK(this->checkCreate();
        IsMessageCreated_ = false;
        IsMessageFinalised_ = true;
    )
    if (MessageType_ == MessageType::REQUEST || MessageType_ == MessageType::NOTIFICATION)
    {
        if (HasParams_) Writer_.EndObject();
        if (MessageType_ == MessageType::REQUEST)
        {
            Writer_.Key("id");
            Writer_.Uint(++RequestID_);
        }
    }
    else // if (MessageType_ == MessageType::RESULT || MessageType_ == MessageType::ERROR)
    {
        Writer_.Key("id");
        if (_ReqID != 0)
            Writer_.Uint(_ReqID);
        else
            Writer_.Null();
    }
    Writer_.EndObject();
    HasParams_ = false;
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

void JsonManager::checkParamBegin()
{
    if (!HasParams_)
    {
        Writer_.Key("params");
        Writer_.StartObject();
        HasParams_ = true;
    }
}

DBLK(
    void JsonManager::checkCreate()
    {
        auto& Messages = Reg_.ctx<MessageHandler>();
        if (!IsMessageCreated_)
            Messages.report("jsn", "No message created", MessageHandler::ERROR);
    }
)
