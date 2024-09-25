#include "libs/bot.h"

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
            // std::cout << "now: " << boost::gregorian::to_iso_extended_string(now) << " parsed: " << boost::gregorian::to_iso_extended_string(parsed_date) << "\n";

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


std::string get_keyboard(std::string mode, std::string session_id){
    std::string reply_markup = "";
    Keyboard keyboard;
    if (mode == "Меню"){
        std::vector<Row> rows;
        Row row1;
        Row row2;
        Row row3;

        InlineKeyboardButton button1;
        button1.text = "Посмотреть расписание";
        button1.callback_data = "schedule|"+session_id;

        InlineKeyboardButton button2;
        button2.text = "Поменять роль";
        button2.callback_data = "change_role|"+session_id;

        InlineKeyboardButton button3;
        button3.text = "Информация";
        button3.callback_data = "info|"+session_id;

        row1.buttons.push_back(button1);
        row2.buttons.push_back(button2);
        row3.buttons.push_back(button3);

        rows.push_back(row1);
        rows.push_back(row2);
        rows.push_back(row3);

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

void onCallback(nlohmann::json update){
    std::string callback_data = update["callback_query"]["data"];
    int64_t chat_id = update["callback_query"]["message"]["chat"]["id"];
    int64_t bot_message_id = update["callback_query"]["message"]["message_id"];


    std::string action = get_action(callback_data);
    
    std::cout << "";

    std::string session_id = get_session_id(callback_data);

    std::ifstream f("session/" + session_id + ".json");
    nlohmann::json session = nlohmann::json::parse(f);
    int page = session["page"];
    int64_t user_id_int = session["user_id"];
    std::string user_id = std::to_string(user_id_int);
    f.close();

    std::ifstream userf("roles/roles.json");
    nlohmann::json user = nlohmann::json::parse(userf);
    userf.close();
    // int time_zone = user[user_id]["time_zone"];


    if (action.find("set_group_") != -1){
        std::string group = callback_data;
        group = group[10];

        nlohmann::json user_data;
        nlohmann::json user_d;
        user_d["group"] = group;
        user_d["time_zone"] = 3;

        user_data[std::to_string(user_id_int)] = user_d;
        user = user_data;

        auto keyboard = get_keyboard("ВыборЧасовогоПояса", session_id);
        bot.edit_message(chat_id, "Выбери часовой пояс: 3", bot_message_id, keyboard);
    
    }else if(action == "time_backward" or action == "time_forward"){
        int time_zone = user[user_id]["time_zone"];

        if (action == "time_forward") time_zone++;
        else time_zone--;
        user[user_id]["time_zone"] = time_zone;

        auto keyboard = get_keyboard("ВыборЧасовогоПояса", session_id);
        bot.edit_message(chat_id, "Выбери часовой пояс: " + std::to_string(time_zone), bot_message_id, keyboard);
    

    } else if (action == "menu"){
        auto keyboard = get_keyboard("Меню", session_id);
        bot.edit_message(chat_id, "Выбери действие", bot_message_id, keyboard);
    } else if(action == "time_select"){
        auto keyboard = get_keyboard("Меню", session_id);
        bot.edit_message(chat_id, "Выбери действие", bot_message_id, keyboard);

    } else if(action == "change_role"){
        auto keyboard = get_keyboard("Регистрация", session_id);
        bot.edit_message(chat_id, "Выбери группу", bot_message_id, keyboard);

    } else if(action == "schedule" or action == "schedule_backward" or action == "schedule_forward"){
        std::string user_group = user[user_id]["group"];
        if (user_group == "1") user_group = "РПО 2 курс";
        else if (user_group == "2") user_group = "ТМ1124";
        if (action == "schedule_backward") page--;
        else if (action == "schedule_forward") page++;
        else page = get_page_by_date(user_group);
        session["page"] = page;

        int time_zone = user[user_id]["time_zone"];
        std::string schedule = get_schedule_by_page(user_group, page, time_zone);
        
        auto keyboard = get_keyboard("МенюРасписание", session_id);
        bot.edit_message(chat_id, schedule, bot_message_id, keyboard);
    } else if (action == "info"){
        auto keyboard = get_keyboard("Назад", session_id);
        bot.edit_message(chat_id, "Тут инфа о проекте", bot_message_id, keyboard);
    }

    // save session
    std::ofstream file_in;
    file_in.open("session/" + session_id + ".json");
    file_in << session.dump();
    file_in.close();

    // save user
    file_in.open("roles/roles.json");
    file_in << user.dump();
    file_in.close();
}

void onAnyMessage (nlohmann::json update){
    int64_t chat_id = update["message"]["chat"]["id"];
    int64_t message_id = update["message"]["message_id"];
    std::string message_text = update["message"]["text"];
    int64_t user_id_int = update["message"]["from"]["id"];
    std::string user_id_str = std::to_string(user_id_int);

    // creating the session
    std::ifstream f("session/session_counter.json");
    nlohmann::json json = nlohmann::json::parse(f);
    f.close();

    int counter = json["last_session"];
    counter++;
    json["last_session"] = counter;

    nlohmann::json session;
    session["user_id"] = user_id_int;
    session["is_group"] = chat_id < 0;
    session["page"] = 1;


    std::ofstream f_out("session/session_counter.json");
    f_out << json.dump();
    f_out.close();

    std::ifstream roles("roles/roles.json");
    nlohmann::json roles_json = nlohmann::json::parse(roles);
    f.close();
    if (message_text == "ботик" || message_text == "Ботик"){
        if (!roles_json[user_id_str].is_null()){
            std::string keyboard = get_keyboard("Меню", std::to_string(counter));
            bot.send_message(chat_id, "в разработке пока что", keyboard);
        }
    
        else{
            if (chat_id < 0){ // то есть группа
                bot.send_message(chat_id, "Тебя нет в базе, напиши мне в лс и выбери свою группу");
            } 
            else {
                auto keyboard = get_keyboard("Регистрация", std::to_string(counter));
                bot.send_message(chat_id, "Тебя нет в базе, выбери свою группу", keyboard);
            }
        }
    }

    if (message_text == "/update"){
        // check the time
        std::ifstream f("session/session_counter.json");
        nlohmann::json session_info = nlohmann::json::parse(f);
        int64_t time_stamp = session_info["last_update"];
        f.close();

        const auto p1 = std::chrono::system_clock::now();
        const auto time_stamp_now = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
        int delay = 60;

        if (time_stamp_now - time_stamp > delay){
            bot.send_message(chat_id, "Начался парсинг расписания");
            bot.update_schedule();
            bot.send_message(chat_id, "Расписание обновлено");
            // save last update
            std::ofstream f_out("session/session_counter.json");
            session_info["last_update"] = time_stamp_now;
            f_out << session_info.dump();
            f_out.close();

        } else {
            bot.send_message(chat_id, "Подожди " + std::to_string((int64_t)delay-(time_stamp_now - time_stamp)) + " секунд");
        }

    }

    // save session
    std::ofstream file_in;
    file_in.open("session/" + std::to_string(counter) + ".json");
    file_in << session.dump();
    file_in.close();

    // save user
    file_in.open("roles/roles.json");
    roles_json[user_id_str]["time_zone"] = 3;
    file_in << roles_json.dump();
    file_in.close();
}

int main() {
    int64_t last_update_id = -1;

    bot.set_functions(onAnyMessage);
    bot.set_functions(onCallback);
    bot.start();
}