#include "libs/tgbot/bot.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <string>
#include <fstream>
#include "utils.h"

[[nodiscard]] int get_page_by_date(std::string group){
    boost::gregorian::date now = boost::posix_time::second_clock::local_time().date();
    boost::gregorian::date_duration one_day(1);

    std::ifstream schedule_file("/home/almas/Рабочий стол/telegramapicpp/lessons/lessons.json");
    nlohmann::json schedule = nlohmann::json::parse(schedule_file);
    std::string formatted_date = boost::gregorian::to_iso_extended_string(now);

    size_t size = schedule[group].size();
    std::string weekdays[7] = {"Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};
    if (group == "ТМ1124"){
        for (int i = 1; i < 10; i++){
            std::string page = "page_" + std::to_string(i);
            if (schedule[group][page][0]["date"] == weekdays[now.day_of_week()-1])
                return i;
        }
    } else
    //binnary search
    for (int i = 0; i < 10; i++){
        int r = size;
        int l = 0;
        while (l <= r){
            int mid = l + (r - l) / 2;

            std::string page = "page_" + std::to_string(mid);
            if (schedule[group][page][0]["date"] == formatted_date){
                return mid;
            }
            auto parsed_date = boost::gregorian::from_simple_string(schedule[group][page][0]["date"]);

            if(parsed_date < now) {
                l = mid + 1;
            }
            else {
                r = mid - 1;
            }
        }
        now += one_day;
        formatted_date = boost::gregorian::to_iso_extended_string(now);
    }
    return 1;
}
[[nodiscard]] std::string get_schedule_by_page(std::string group, int page, int time_zone){
    std::ifstream schedule_file("/home/almas/Рабочий стол/telegramapicpp/lessons/lessons.json");
    nlohmann::json schedule = nlohmann::json::parse(schedule_file);
    std::string page_str = "page_" + std::to_string(page);
    if (schedule[group][page_str].is_null()) return "Нет расписания на " + page_str;

    std::string date = schedule[group][page_str][0]["date"];
    std::string schedule_str = "Расписание на " + date + " для " + group + '\n' + "<blockquote>";
    if (group == "ТМ1124"){
        time_zone = time_zone - 5;
    } else{
        time_zone = time_zone - 3;
    }

    for (int subject = 0; subject < 10; ++subject){
        if (schedule[group][page_str][subject]["subject_name"].is_null()) break;
        schedule_str += std::to_string(subject + 1) + ") <b>";

        schedule_str += schedule[group][page_str][subject]["subject_name"];
        schedule_str += "</b>\n";

        std::string time_string = schedule[group][page_str][subject]["started_at"];
        boost::posix_time::time_duration parsed_time = boost::posix_time::duration_from_string(time_string);
        parsed_time += boost::posix_time::hours(time_zone);

        schedule_str += (parsed_time.hours() < 10 ? "0" + std::to_string(parsed_time.hours()) : std::to_string(parsed_time.hours()));
        schedule_str += ":";
        schedule_str += (parsed_time.minutes() == 0 ? "00" : std::to_string(parsed_time.minutes()));
        schedule_str += "-";

        time_string = schedule[group][page_str][subject]["finished_at"];
        parsed_time = boost::posix_time::duration_from_string(time_string);

        parsed_time += boost::posix_time::hours(time_zone);
        schedule_str += (parsed_time.hours() < 10 ? "0" + std::to_string(parsed_time.hours()) : std::to_string(parsed_time.hours()));
        schedule_str += ":";
        schedule_str += (parsed_time.minutes() < 0 ? "00" : std::to_string(parsed_time.minutes()));

        if(group == "ТМ1124"){
            schedule_str += " каб. ";
            schedule_str += schedule[group][page_str][subject]["room"];
        }
        schedule_str += '\n';
        schedule_str += schedule[group][page_str][subject]["teacher_name"];
        schedule_str += + "\n\n";
    }
    schedule_str += "</blockquote>";
    return schedule_str;
}

static telegrambot::tgbot bot(token);

bool isAdmin(int64_t user_id){
    if (user_id == 6560101088) return true;
    return false;
}

std::string get_keyboard(std::string mode, std::string session_id, bool admin, int extra = 0){
    std::string reply_markup = "";
    Keyboard keyboard;
    if (mode == "Меню"){
        std::vector<Row> rows;
        Row row1;
        Row row2;
        Row row3;
        Row row4;

        InlineKeyboardButton button1;
        button1.text = "Посмотреть расписание";
        button1.callback_data = "schedule|"+session_id;

        InlineKeyboardButton button2;
        button2.text = "Поменять роль";
        button2.callback_data = "change_role|"+session_id;

        InlineKeyboardButton button3;
        button3.text = "Информация";
        button3.callback_data = "info|"+session_id;

        InlineKeyboardButton button4;
        button4.text = "Настройки чата";
        button4.callback_data = "chat_settings|"+session_id;

        row1.buttons.push_back(button1);
        row2.buttons.push_back(button2);
        row3.buttons.push_back(button3);
        row4.buttons.push_back(button4);

        rows.push_back(row1);
        rows.push_back(row2);
        rows.push_back(row3);

        if (admin){
            Row row1;
            InlineKeyboardButton button1;
            button1.text = "Дать показания";
            button1.callback_data = "get_info|"+session_id;
            row1.buttons.push_back(button1);
            rows.push_back(row1);
        }

        return keyboard.to_string(rows);


    } else if (mode == "Регистрация"){
        std::vector<Row> rows;
        Row row1;

        InlineKeyboardButton button1;
        button1.text = "РПО 1 курс";
        button1.callback_data = "set_group_1|"+session_id;

        InlineKeyboardButton button2;
        button2.text = "ТМ1124";
        button2.callback_data = "set_group_2|"+session_id;


        row1.buttons.push_back(button1);
        row1.buttons.push_back(button2);

        rows.push_back(row1);

        return keyboard.to_string(rows);

    } else if (mode == "ВыборЧасовогоПояса"){
        std::vector<Row> rows;
        Row row1;

        InlineKeyboardButton button1;
        button1.text = "<-";
        button1.callback_data = "time_backward|"+session_id;

        InlineKeyboardButton button2;
        button2.text = "Выбрать";
        button2.callback_data = "time_select|"+session_id;

        InlineKeyboardButton button3;
        button3.text = "->";
        button3.callback_data = "time_forward|"+session_id;

        row1.buttons.push_back(button1);
        row1.buttons.push_back(button2);
        row1.buttons.push_back(button3);

        rows.push_back(row1);

        return keyboard.to_string(rows);

    } else if (mode == "МенюРасписание"){
        std::vector<Row> rows;
        Row row1;

        InlineKeyboardButton button1;
        button1.text = "Назад";
        button1.callback_data = "schedule_backward|"+session_id;

        InlineKeyboardButton button2;
        button2.text = "Меню";
        button2.callback_data = "menu|"+session_id;

        InlineKeyboardButton button3;
        button3.text = "Вперед";
        button3.callback_data = "schedule_forward|"+session_id;

        row1.buttons.push_back(button1);
        row1.buttons.push_back(button2);
        row1.buttons.push_back(button3);

        rows.push_back(row1);

        return keyboard.to_string(rows);
    } else if (mode == "Назад"){
        std::vector<Row> rows;
        Row row1;

        InlineKeyboardButton button1;
        button1.text = "Назад";
        button1.callback_data = "menu|"+session_id;

        row1.buttons.push_back(button1);

        rows.push_back(row1);

        return keyboard.to_string(rows);
    } else if (mode == "МенюПоказания"){
        std::vector<Row> rows;
        Row row1;

        InlineKeyboardButton button1;
        button1.text = "Номер недели";
        button1.callback_data = "get_week|"+session_id;

        row1.buttons.push_back(button1);

        rows.push_back(row1);
        return keyboard.to_string(rows);
    } else if (mode == "МенюНастроекЧата"){
        std::vector<Row> rows;
        Row row1;

        InlineKeyboardButton button1;
        if (extra == 1){ // заход выход = true // ✅❌
            button1.text = "Заход-выход ✅";
        } else if (extra == 2){
            button1.text = "Заход-выход ❌";
        }
        button1.callback_data = "chat_settings|"+session_id;
        row1.buttons.push_back(button1);

        rows.push_back(row1);
        return keyboard.to_string(rows);
    }
    return "";
}

std::string get_action(std::string callback){
    bool found;
    std::string result{};
    for (char ch : callback){
        if (ch == '|') found = true;
        if (!found) result += ch;
    }
    return result;
}

std::string get_session_id(std::string callback){
    bool found;
    std::string result{};
    for (char ch : callback){
        if (found) result += ch;
        if (ch == '|') found = true;
    }
    return result;
}

void onCallback(Callback_query::Ptr callback_query){
    std::string action = get_action(callback_query->data);
    std::string session_id = get_session_id(callback_query->data);

    std::ifstream f("session/sessions.json");
    nlohmann::json sessions = nlohmann::json::parse(f);
    nlohmann::json session = sessions[std::to_string(callback_query->message->chat->id)];
    
    int page = session["page"];
    std::string user_id = std::to_string(callback_query->from->id);
    f.close();

    std::ifstream userf("roles/roles.json");
    nlohmann::json user = nlohmann::json::parse(userf);
    userf.close();

    if (user[user_id].is_null())
    {
        std::string user_name = callback_query->from->first_name;
        bot.getApi().edit_message(callback_query->message->chat->id, user_name + ", ты нажал(а) на кнопку и я не знаю под какую группу мне выдавать расписание", callback_query->message->message_id);
        return;
    }
    // int time_zone = user[user_id]["time_zone"];


    if (action.find("set_group_") != -1){
        std::string group = callback_query->data;
        group = group[10];
        
        int time_zone = user[std::to_string(callback_query->from->id)]["time_zone"];

        nlohmann::json user_data;
        user_data["group"] = group;
        user_data["time_zone"] = time_zone;

        user[std::to_string(callback_query->from->id)] = user_data;

        auto keyboard = get_keyboard("ВыборЧасовогоПояса", session_id, isAdmin(callback_query->from->id));
        bot.getApi().edit_message(callback_query->message->chat->id, "Выбери часовой пояс: " + std::to_string(time_zone), callback_query->message->message_id, keyboard);
    
    }else if(action == "time_backward" or action == "time_forward"){
        int time_zone = user[user_id]["time_zone"];

        if (action == "time_forward") time_zone++;
        else time_zone--;
        user[user_id]["time_zone"] = time_zone;

        auto keyboard = get_keyboard("ВыборЧасовогоПояса", session_id, isAdmin(callback_query->from->id));
        bot.getApi().edit_message(callback_query->message->chat->id, "Выбери часовой пояс: " + std::to_string(time_zone), callback_query->message->message_id, keyboard);
    

    } else if (action == "menu"){
        auto keyboard = get_keyboard("Меню", session_id, isAdmin(callback_query->from->id));
        bot.getApi().edit_message(callback_query->message->chat->id, "Выбери действие", callback_query->message->message_id, keyboard);
    } else if(action == "time_select"){
        auto keyboard = get_keyboard("Меню", session_id, isAdmin(callback_query->from->id));
        bot.getApi().edit_message(callback_query->message->chat->id, "Выбери действие", callback_query->message->message_id, keyboard);

    } else if(action == "change_role"){
        auto keyboard = get_keyboard("Регистрация", session_id, isAdmin(callback_query->from->id));
        bot.getApi().edit_message(callback_query->message->chat->id, "Выбери группу", callback_query->message->message_id, keyboard);

    } else if(action == "schedule" or action == "schedule_backward" or action == "schedule_forward"){
        std::string user_group = user[user_id]["group"];

        if (user_group == "1"){
            user_group = "РПО 2 курс";
        }
        else if (user_group == "2"){
            user_group = "ТМ1124";
        }
        
        if (action == "schedule_backward"){
            --page;
        }
        else if (action == "schedule_forward"){
            ++page;
        } else {
            page = get_page_by_date(user_group);
        }

        session["page"] = page;

        int time_zone = user[user_id]["time_zone"];
        std::string schedule = get_schedule_by_page(user_group, page, time_zone);
        
        auto keyboard = get_keyboard("МенюРасписание", session_id, isAdmin(callback_query->from->id));
        bot.getApi().edit_message(callback_query->message->chat->id, schedule, callback_query->message->message_id, keyboard);
    } else if (action == "info"){
        auto keyboard = get_keyboard("Назад", session_id, isAdmin(callback_query->from->id));
        bot.getApi().edit_message(callback_query->message->chat->id, "О проекте:\n\n-Статус: в разработке\n-Бот написан на языке C++\n-Парсер написан на пайтоне\n\n                       о багах сюда ⬎\nРазработчик: @e11673f48c5b0ad057eebd44ee444ce6", callback_query->message->message_id, keyboard);
    } else if (action == "get_info"){
        auto keyboard = get_keyboard("МенюПоказания", session_id, isAdmin(callback_query->from->id));
        bot.getApi().edit_message(callback_query->message->chat->id, "Выбери что хочешь изменить", callback_query->message->message_id, keyboard);
    } else if(action == "get_week"){
        session["action"] = "get_week";
        bot.getApi().edit_message(callback_query->message->chat->id, "Напиши номер недели", callback_query->message->message_id);
    } else if (action == "chat_settings"){
        nlohmann::json chat_settings = session["chat_settings"];
        bool in_out = chat_settings["in_out"];
        int extra = 2;
        if (in_out == true){
            extra = 1;
        }

        auto keyboard = get_keyboard("МенюНастроекЧата", session_id, false, extra);
        bot.getApi().edit_message(callback_query->message->chat->id, "Выбери раздел", callback_query->message->message_id, keyboard);
    }
    // save session
    std::ofstream file_in;
    file_in.open("session/sessions.json");
    sessions[std::to_string(callback_query->message->chat->id)] = session;
    file_in << sessions.dump();
    file_in.close();

    // save user
    file_in.open("roles/roles.json");
    file_in << user.dump();
    file_in.close();
}

void onAnyMessage (Message::Ptr message){
    std::string chat_id_str = std::to_string(message->chat->id);
    std::string user_id_str = std::to_string(message->from->id);
    
    std::ifstream roles("roles/roles.json");
    nlohmann::json roles_json = nlohmann::json::parse(roles);

    std::ifstream f("session/sessions_info.json");
    nlohmann::json sessions_info = nlohmann::json::parse(f);
    f.close();

    // load a session
    std::ifstream file_in;
    file_in.open("session/sessions.json");
    nlohmann::json sessions = nlohmann::json::parse(file_in);
    file_in.close();

    if (!sessions[chat_id_str]["action"].is_null()){
        std::string action = sessions[chat_id_str]["action"];

        // check action
        if (action != "" && sessions[chat_id_str]["user_id"] == message->from->id){
            if (action == "get_week"){
                std::string path_to_config = "/home/almas/Рабочий стол/telegramapicpp/utils/config.json";
                std::ifstream f(path_to_config);
                nlohmann::json config = nlohmann::json::parse(f);
                f.close();

                std::ofstream f_out(path_to_config);
                config["parser_week"] = std::stoi(message->text);
                f_out << config.dump();
                f_out.close();
                std::string keyboard = get_keyboard("Назад", chat_id_str, isAdmin(message->from->id));
                bot.getApi().send_message(message->chat->id, "Успешно", keyboard);
                sessions[chat_id_str]["action"] = "";
            }
        }
    }


    if (message->text == "ботик" || message->text == "Ботик"){
        // creating a session
        nlohmann::json session;
        session["page"] = 1;
        session["user_id"] = message->from->id;
        session["action"] = "";
        // nlohmann::json chat_settings;
        // chat_settings["in_out"] = true;
        // session["chat_settings"] = chat_settings;
        sessions[chat_id_str] = session;


        if (!roles_json[user_id_str].is_null()){
            std::string keyboard = get_keyboard("Меню", chat_id_str, isAdmin(message->from->id));
            bot.getApi().send_message(message->chat->id, "Выбери действие", keyboard);
        }
        else{
            roles_json[user_id_str]["time_zone"] = 3;
            auto keyboard = get_keyboard("Регистрация", chat_id_str, isAdmin(message->from->id));
            bot.getApi().send_message(message->chat->id, "Тебя нет в базе, выбери свою группу", keyboard);
        }
    }

    if (message->text == "/update"){
        // check the time
        int64_t time_stamp = sessions_info["last_update"];

        const auto time_stamp_now = std::time({});
        int delay = 300; // 5 minutes

        if (time_stamp_now - time_stamp > delay){
            bot.getApi().send_message(message->chat->id, "Начался парсинг расписания");
            bot.update_schedule();
            bot.getApi().send_message(message->chat->id, "Расписание обновлено");
            
            // save last update
            sessions_info["last_update"] = time_stamp_now;

        } else {
            bot.getApi().send_message(message->chat->id, "Подожди " + std::to_string((int64_t)delay-(time_stamp_now - time_stamp)) + " секунд");
        }

    }

    // save a session
    std::ofstream file_out;
    file_out.open("session/sessions.json");
    file_out << sessions.dump();
    file_out.close();

    // save sessions info
    std::ofstream f_out("session/sessions_info.json");
    f_out << sessions_info.dump();
    f_out.close();

    
    // save user
    f_out.open("roles/roles.json");
    f_out << roles_json.dump();
    f_out.close();
}

// void onLeft(nlohmann::json update)
// {
//     int64_t chat_id = update["message"]["chat"]["id"];
//     bot.getApi().send_message(chat_id, "press f");
// }

// void onJoin(nlohmann::json update){
//     int64_t chat_id = update["message"]["chat"]["id"];
//     bot.getApi().send_message(chat_id, "Дарова, ", update["message"]["new_chat_member"]["first_name"]);
// }

int main()
{
    bot.set_onAnyMessage(onAnyMessage);
    bot.set_onCallback(onCallback);
    // bot.set_onLeft(onLeft);
    // bot.set_onJoin(onJoin);

    bot.start();
}