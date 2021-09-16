#ifndef JSON_MANAGER_HPP
#define JSON_MANAGER_HPP

#include <string>
#include <vector>

#include <entt/entity/registry.hpp>
#include <rapidjson/stringbuffer.h>
#ifdef NDEBUG
        #include <rapidjson/writer.h>
#else
        #include <rapidjson/prettywriter.h>
#endif

#include "message_handler.hpp"
#include "network_message.hpp"

using namespace rapidjson;

class JsonManager
{

    public:

        using ClientIDType = entt::entity;
        // Request ID should be unique, but is simply an increased integer
        // So after running more than a year with 100 requests per second
        // there will occur an overflow
        // If changed to another type such as int64, another method from
        // rapidjson's writer has to be chosen
        using RequestIDType = std::uint32_t;

        enum class ErrorType : int
        {
            METHOD,
            PARAMS,
            PARSE,
            REQUEST
        };
        enum class ParamsType : int
        {
            NUMBER,
            STRING
        };

        struct ParamCheckResult
        {
            bool Success{true};
            ErrorType Error{ErrorType::METHOD};
            ClientIDType ClientID{0};
            RequestIDType RequestID{0};
        };

        explicit JsonManager(entt::registry& _Reg) : Reg_(_Reg) {}

        JsonManager& createNotification(const std::string& _Notification);
        JsonManager& createRequest(const std::string& _Req);

        // Errors
        JsonManager& createError(ErrorType _e);

        // Single value results
        JsonManager& createResult(const bool _r);
        JsonManager& createResult(const char* _r);
        JsonManager& createResult(const std::string& _r);

        // Structured results (has to be followed by "array" or "object")
        JsonManager& createResult();

        JsonManager& beginArray();
        JsonManager& beginArray(const char* _Key);
        JsonManager& endArray();

        JsonManager& beginObject();
        JsonManager& endObject();

        JsonManager& addParam(const std::string& _Name, bool _v);
        JsonManager& addParam(const std::string& _Name, double _v);
        JsonManager& addParam(const std::string& _Name, std::uint32_t _v);
        JsonManager& addParam(const std::string& _Name, std::uint64_t _v);
        JsonManager& addParam(const std::string& _Name, const char* _v);
        JsonManager& addParam(const std::string& _Name, const std::string& _v);

        JsonManager& addValue(double _v);
        JsonManager& addValue(std::uint32_t _v);
        JsonManager& addValue(int _v);
        JsonManager& addValue(const char* _v);

        ParamCheckResult checkParams(const NetworkDocument& _d, std::vector<ParamsType> _p);

        void finalise(RequestIDType _ReqID = 0);
        RequestIDType getRequestID() const {return RequestID_;}
        const char* getString() const {return Buffer_.GetString();}

        static auto getParams(const NetworkDocument& _d)
        {
            return (*_d.Payload)["params"].GetArray();
        }

    private:

        enum class MessageType
        {
            ERROR,
            NOTIFICATION,
            REQUEST,
            RESULT
        };

        void createHeaderJsonRcp();
        void createHeaderNotificationRequest(const std::string& _m);
        void createHeaderError();
        void createHeaderResult();

        void checkParamBegin();
        DBLK(
            void checkCreate();
        )

        entt::registry& Reg_;

        StringBuffer Buffer_;
        #ifdef NDEBUG
                Writer<StringBuffer> Writer_{Buffer_};
        #else
                PrettyWriter<StringBuffer> Writer_{Buffer_};
        #endif
        MessageType MessageType_{MessageType::REQUEST};

        DBLK(
            bool IsMessageFinalised_{true};
            bool IsMessageCreated_{false};
        )

        std::string Params_;
        bool HasParams_{false};

        RequestIDType RequestID_{0};
};

#endif // JSON_MANAGER_HPP
