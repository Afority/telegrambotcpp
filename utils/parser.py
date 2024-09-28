import os
import pandas as pd
import requests
import time
import datetime
import json
import time

work_directory = "/home/almas/Рабочий стол/telegramapicpp/utils/"

def logging(mode, text):
    with open(work_directory+"errors.txt", "a", encoding="UTF-8") as file:
        file.write(f"\n[{mode}] {datetime.datetime.now()} {text}")

def get_xlsx():
    headers = {
        'accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7',
        'accept-language': 'ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7',
        # 'cookie': 'COMPASS=apps-spreadsheets=CmUACWuJVxtO8F5TgZ-UhUeuQUs0-NSm0l1bl77TJlmITsTqOAMNmIYAAHy5sN9NJY0-ei5P3piOcmlMES7QDtM0k2dISHoz_P8Xtt3YR-Z8X1OM7NvFq2AChKLt4Y3xzOhCNzY-ExCIn7C3Bhp2AAlriVfCrxoVTWGjaJX0AhfHTC0wl3tSMGuJvoBujyVT03xdTIxikbKPwjt5bxZeFs7uPID4nQcHtubHweE-HIWI5xQouH5Pvs8V8K8Py_bn5DaUJwDUgwUQc-hpvQyqUKj45xWMtUsIy4sDTiCQ0uOVgxZf-A==; AEC=AVYB7cpd6Kb3fu_6pqLN4VXYmudZwGWjUIpGLjjjlT72a3jiwlSf65AXdww; NID=517=AE4uc9DVRp_0aiN8-3wrQLn1ey94UAVOMH4sGUE665TG_OX0q_sMewMNPDPQtkyCQcpQ9RWLMUEae_vgG238mQbElMJltTxY_7pi8zqmf71Uh0VpgfkPP04SpCCkyQiEXIQ9SZwaxLwFU1zobS-4JvRSWFO5QaDZXWZdtV-e3goOBjMwIv94mNVud1MRVMiRyOHLOxImrtcgdc8ICF9jCV0S8qaZlKzLMH4ZuEkuMx02zc1zvIkMl5C_DPs',
        'priority': 'u=0, i',
        'referer': 'https://docs.google.com/spreadsheets/d/1yEz1J_mavcX89rOJ5eeCZL85pM7UCqeUIwNS2BgKnuw/edit?gid=557751957',
        'sec-ch-ua': '"Chromium";v="128", "Not;A=Brand";v="24", "Google Chrome";v="128"',
        'sec-ch-ua-arch': '"x86"',
        'sec-ch-ua-bitness': '"64"',
        'sec-ch-ua-form-factors': '"Desktop"',
        'sec-ch-ua-full-version': '"128.0.6613.137"',
        'sec-ch-ua-full-version-list': '"Chromium";v="128.0.6613.137", "Not;A=Brand";v="24.0.0.0", "Google Chrome";v="128.0.6613.137"',
        'sec-ch-ua-mobile': '?0',
        'sec-ch-ua-model': '""',
        'sec-ch-ua-platform': '"Linux"',
        'sec-ch-ua-platform-version': '"6.8.0"',
        'sec-ch-ua-wow64': '?0',
        'sec-fetch-dest': 'iframe',
        'sec-fetch-mode': 'navigate',
        'sec-fetch-site': 'same-origin',
        'sec-fetch-user': '?1',
        'upgrade-insecure-requests': '1',
        'user-agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36',
        'x-client-data': 'CJa2yQEIo7bJAQipncoBCMWBywEIk6HLAQic/swBCIegzQEI/KnOAQjEts4BCKO7zgEI2b3OAQjcvs4BGPbJzQEYnbHOAQ==',
    }

    params = {
        'format': 'xlsx',
        'id': '1yEz1J_mavcX89rOJ5eeCZL85pM7UCqeUIwNS2BgKnuw',
    }

    response = requests.get(
        'https://docs.google.com/spreadsheets/d/1yEz1J_mavcX89rOJ5eeCZL85pM7UCqeUIwNS2BgKnuw/export',
        params=params,
        headers=headers,
    )
    # save
    with open(work_directory+"1.xlsx", "wb") as file:
        file.write(response.content)

def get_schedule2():
    get_xlsx()
    with open("/home/almas/Рабочий стол/telegramapicpp/lessons/lessons.json", 'r', encoding="UTF-8") as file:
        lessons = json.load(file)
    # sharaga 2
    df = pd.read_excel(work_directory+"1.xlsx", sheet_name="ТМ(9) и СП")
    index = 1
    group = "ТМ1124"
    lessons[group] = {}
    for i in range(123):
        if df.columns[i] == group:
            index = i
            break
    # ищем индекс недели
    with open(work_directory+"config.json") as file:
        config = json.load(file)
    week = config.get("parser_week", 1)
    
    for i in range(index, 10):
        columns = [df.columns[i]]
        schedule = df[columns]
        if (schedule.values[0][0] == week):
            index_week = i
            break
    else:
        raise TypeError("Не найдено расписание")
    
    columns = [df.columns[1], df.columns[index_week], df.columns[2], df.columns[0]]
    schedule = df[columns]
    
    lastweek = ""
    counter = 0
    for i in schedule.values[1:]:
        time:str = i[0]
        subject_name = i[1]
        weekday = i[-1]
        if isinstance(weekday, float):
            weekday = lastweek
        else:
            lastweek = i[-1]
            counter += 1
        
        
        if (isinstance(subject_name, str)):
            started_at, finished_at = time.split("-")
            data = subject_name.split("\n")
            type_ = data[0]
            object_ = data[1]
            teacher_name = data[2]
            room = ""
            try:
                room = data[3]
            except:
                ...
            if (not lessons[group].get(f"page_{counter}")):
                lessons[group][f"page_{counter}"] = []
            
            lessons[group][f"page_{counter}"].append({
                    "date":weekday.strip(),
                    "started_at":started_at,
                    "finished_at":finished_at,
                    "subject_name":type_ + " " + object_,
                    "teacher_name":teacher_name,
                    "room":room
                })
            
            
            # result[group][f"page_{counter}"][weekday][time] = subject_name.replace("\n", " ")
            
        
    
    with open(f"/home/almas/Рабочий стол/telegramapicpp/lessons/lessons.json", "w", encoding="UTF-8") as file:
        json.dump(lessons, file, ensure_ascii=False)

def get_schedule1(login, password, group):    
    headers = {
        'User-Agent': 'Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:123.0) Gecko/20100101 Firefox/123.0',
        'Accept': 'application/json, text/plain, */*',
        'Accept-Language': 'ru_RU, ru',
        # 'Accept-Encoding': 'gzip, deflate, br',
        'Authorization': 'Bearer null',
        'Content-Type': 'application/json',
        'Origin': 'https://journal.top-academy.ru',
        'Connection': 'keep-alive',
        'Referer': 'https://journal.top-academy.ru/',
        'Sec-Fetch-Dest': 'empty',
        'Sec-Fetch-Mode': 'cors',
        'Sec-Fetch-Site': 'same-site',
        # Requests doesn't support trailers
        # 'TE': 'trailers',
    }

    json_data = {
        'application_key': '6a56a5df2667e65aab73ce76d1dd737f7d1faef9c52e8b8c55ac75f565d8e8a6',
        'id_city': None,
        'password': password,
        'username': login,
    }
    
    response = requests.post('https://msapi.top-academy.ru/api/v2/auth/login', headers=headers, json=json_data)
    
    if (response.status_code == 422):
        return logging("ERROR", f"неверные данные для входа, код группы {group}")
    elif (response.status_code != 200):
        return logging("ERROR", f"Неизвестная ошибка")
    
    access_token = str(response.json().get('access_token')).rstrip('None')
    
    
    headers = {
        'authority': 'msapi.top-academy.ru',
        'accept': 'application/json, text/plain, */*',
        'accept-language': 'ru_RU, ru',
        'authorization': f'Bearer {access_token}',
        'origin': 'https://journal.top-academy.ru',
        'referer': 'https://journal.top-academy.ru/',
        'sec-ch-ua': '"Chromium";v="122", "Not(A:Brand";v="24", "Google Chrome";v="122"',
        'sec-ch-ua-mobile': '?0',
        'sec-ch-ua-platform': '"Linux"',
        'sec-fetch-dest': 'empty',
        'sec-fetch-mode': 'cors',
        'sec-fetch-site': 'same-site',
        'user-agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36',
    }
    day = datetime.datetime.strptime(f'1-{(datetime.datetime.now() + datetime.timedelta(days=2)).month}-2024', "%d-%m-%Y")

    lessons = {
        "РПО 2 курс":{},
        "ТМ1124":{}
    }
    days = 0
    curr_month = day.month
    counter_less = 1
    while True:
        days += 1
        if day.month != curr_month:
            break
        params = { # 2024-04-11
            'date_filter': day.strftime('%Y-%m-%d'),
        }
        response = requests.get('https://msapi.top-academy.ru/api/v2/schedule/operations/get-by-date', params=params, headers=headers)
        less_json = response.json()
        if type(less_json) == list and less_json:
            if (not lessons["РПО 2 курс"].get(f"page_{counter_less}")):
                lessons["РПО 2 курс"][f"page_{counter_less}"] = []
                
            for lesson in less_json:
                lessons["РПО 2 курс"][f"page_{counter_less}"].append({
                    "date":lesson["date"],
                    "started_at":lesson["started_at"],
                    "finished_at":lesson["finished_at"],
                    "teacher_name":lesson["teacher_name"],
                    "subject_name":lesson["subject_name"],
                    "room_name":lesson["room_name"].strip()
                })
            counter_less += 1
        day += datetime.timedelta(days=1)
    
    with open(f"/home/almas/Рабочий стол/telegramapicpp/lessons/lessons.json", "w", encoding="UTF-8") as file:
        json.dump(lessons, file, ensure_ascii=False)


if __name__ == "__main__":
    with open(work_directory+"config.json", "r", encoding="UTF-8") as file:
        data: dict = json.load(file)
        for user in data["user"]:
            get_schedule1(login=user[0], password=user[1], group=user[2])
    get_schedule2()
