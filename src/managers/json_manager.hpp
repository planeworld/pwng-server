#ifndef JSON_MANAGER_HPP
#define JSON_MANAGER_HPP

#include <string>

#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

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

        explicit JsonManager(entt::registry& _Reg,
                             moodycamel::ConcurrentQueue<NetworkMessage>* _InputQueue,
                             moodycamel::ConcurrentQueue<NetworkMessage>* _OutputQueue) :
                             Reg_(_Reg),
                             InputQueue_(_InputQueue),
                             OutputQueue_(_OutputQueue){}

        JsonManager& createNotification(const std::string& _Notification);
        JsonManager& createRequest(const std::string& _Req);

        // Single value results
        JsonManager& createResult(const bool _r);
        JsonManager& createResult(const char* _r);
        JsonManager& createResult(const std::string& _r);

        // Structured results (has to be followed by "array" or "object")
        JsonManager& createResult();

        JsonManager& beginObject();
        JsonManager& endObject();

        JsonManager& addParam(const std::string& _Name, bool _v);
        JsonManager& addParam(const std::string& _Name, double _v);
        JsonManager& addParam(const std::string& _Name, std::uint32_t _v);
        JsonManager& addParam(const std::string& _Name, const char* _v);
        JsonManager& addParam(const std::string& _Name, const std::string& _v);
        // JsonManager& addValue(const std::string& _v);
        RequestIDType send(ClientIDType _ClientID, RequestIDType _ReqID = 0);
        const char* getString()
        {
            return Buffer_.GetString();
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
        void createHeaderResult();
        DBLK(
            void checkCreate();
        )

        entt::registry& Reg_;

        moodycamel::ConcurrentQueue<NetworkMessage>* InputQueue_;
        moodycamel::ConcurrentQueue<NetworkMessage>* OutputQueue_;

        StringBuffer Buffer_;
        Writer<StringBuffer> Writer_{Buffer_};
        MessageType MessageType_{MessageType::REQUEST};

        DBLK(
            bool IsMessageSend_{true};
            bool IsMessageCreated_{false};
        )

        std::string Params_;

        RequestIDType RequestID_{0};
};

#endif // JSON_MANAGER_HPP
