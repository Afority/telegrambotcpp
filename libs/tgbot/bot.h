#ifndef __linux__
"качай линукс";
#endif

#ifndef bot_h
#define bot_h

#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <cctype>

#include "../json/json.hpp"

#include "config.h"
#include "types.h"
#include "https_requests.h"
namespace telegrambot{
std::string _token{};
std::string _url = "api.telegram.org";

nlohmann::json _send_request(std::string method){ // method = /getMe
    while (method.find('\n') != -1){
        method.replace(method.find('\n'), 1, "%0A");
    }
    while (method.find('+') != -1){
        method.replace(method.find('+'), 1, "%2B");
    }
    nlohmann::json result = nlohmann::json::parse(send_request(_url,  "/bot" + _token + method).body());


    if (result["ok"] == true){
        return result;
    }
    std::cout << result << "\nok = false\n";

    return nlohmann::json{};
}
class Api{
public:
    nlohmann::json edit_message(int64_t chat_id, std::string text, int64_t message_id, std::string reply_markup = "", std::string parse_mode = "HTML"){
        return _send_request("/editMessageText?chat_id=" + std::to_string(chat_id) + 
        "&text=" + text + 
        "&message_id=" + std::to_string(message_id) + 
        "&reply_markup=" + reply_markup + 
        "&parse_mode=" + parse_mode);
    }
    
    nlohmann::json send_message(int64_t chat_id, std::string text, std::string reply_markup = "", std::string parse_mode = "HTML"){
        return _send_request("/sendMessage?chat_id=" + std::to_string(chat_id) + 
        "&text=" + text + 
        "&reply_markup=" + reply_markup +
        "&parse_mode=" + parse_mode
        );
    }
};

Api api;

class tgbot{
public:
    // set handlers
    void set_onJoin(void (*p)(Message::Ptr))
    {
        _onJoin = p;
    }
    void set_onAnyMessage(void (*p)(Message::Ptr))
    {
        _onAnyMessage = p;
    }
    void set_onCallback(void (*p)(Callback_query::Ptr))
    {
        _onCallback = p;
    }
    void set_onLeft(void (*p)(Message::Ptr))
    {
        _onLeft = p;
    }
    void start(){
        signal(SIGSEGV, [](int s){
            printf("Ошибка чтения переменной\n");
            exit(0);
        });

        signal(SIGINT, [](int s) {
            printf("\nSIGINT got\n");
            exit(0);
        });
        while (true){
            // get updates
            try{
                nlohmann::json updates = _send_request("/getupdates?offset=" + std::to_string(last_update_id + 1));
                for (int index = 0; index < updates["result"].size(); index++){
                    last_update_id = updates["result"][index]["update_id"];
                    std::cout << updates << "\n";
                    handle_update(updates["result"][index]);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(70));
            }
            catch(const std::exception &e){
                std::cout << e.what() << '\n';
            }  
        }
    }
    void update_schedule(){
        system(R"("/home/almas/Рабочий стол/telegramapicpp/utils/work/bin/python" "/home/almas/Рабочий стол/telegramapicpp/utils/parser.py")");
    }
    tgbot(std::string token){
        _token = token;
    }
    Api getApi(){
        return api;
    }
private:
    void (*_onJoin)(Message::Ptr);
    void (*_onAnyMessage)(Message::Ptr);
    void (*_onCallback)(Callback_query::Ptr);
    void (*_onLeft)(Message::Ptr);
    
    int64_t last_update_id{};
    int64_t update_id{};

    void handle_update(nlohmann::json update){
        
        Callback_query::Ptr callback_query(new Callback_query);
        Message::Ptr message(new Message);
        Chat::Ptr chat;
        From::Ptr from;
        From::Ptr from_callback;
        // message->from = from;
        // message->chat = chat;
        std::string mode{};
        if (!update["message"]["text"].is_null()){
            mode = "text";
            message =   get_message_atr(update["message"]);
            chat =      get_chat_atr(update["message"]["chat"]);
            from =      get_from_atr(update["message"]["from"]);

            message->from = from;
            message->chat = chat;
        } else if (!update["callback_query"].is_null()){
            mode = "callback";
            message =       get_message_atr(update["callback_query"]["message"]);
            from_callback = get_from_atr(update["callback_query"]["from"]); // callback
            from =          get_from_atr(update["callback_query"]["message"]["from"]); // message
            chat =          get_chat_atr(update["callback_query"]["message"]["chat"]);
            message->chat = chat;
            message->from = from;

            callback_query->from = from_callback;
            callback_query->message = message;

            callback_query->data = update["callback_query"]["data"];
        }

        if (_onAnyMessage && mode == "text"){
            _onAnyMessage(message); // handle text
        }
        if (_onAnyMessage && mode == "callback"){
            _onCallback(callback_query); // handle callback's
        }
    }
    
};
}

#endif // !defined bot_h