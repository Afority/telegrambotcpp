#ifndef types_h
#define types_h

#include <memory>
#include <string>
#include "../json/json.hpp"

struct From{
    typedef std::shared_ptr<From> Ptr;
    int64_t id;
    std::string first_name;
};

struct Chat{
    typedef std::shared_ptr<Chat> Ptr;
    int64_t id;
    std::string title;
};

struct Message{
    typedef std::shared_ptr<Message> Ptr;

    std::string text;
    int64_t message_id;

    std::shared_ptr<Chat> chat;
    std::shared_ptr<From> from;
};
struct Callback_query{
    typedef std::shared_ptr<Callback_query> Ptr;
    std::shared_ptr<Message> message;
    std::shared_ptr<From> from;
    std::string data;
};
Message::Ptr get_message_atr(nlohmann::json update){
    Message::Ptr message(new Message);

    message->text = update["text"];
    message->message_id = update["message_id"];
    return message;
}
From::Ptr get_from_atr(nlohmann::json update){
    From::Ptr from(new From);
    from->first_name = update["first_name"];
    from->id = update["id"];
    return from;
}
Chat::Ptr get_chat_atr(nlohmann::json update){
    Chat::Ptr chat(new Chat);
    chat->id = update["id"];
    if (!update["title"].is_null()){
        chat->title = update["title"];
    }
    return chat;
}

#endif // !defined types_h