# -*- coding: utf-8 -*-
import json
from urllib import parse
from io import BytesIO
from config import *

import requests
from PIL import Image
from fake_useragent import UserAgent

# 禁用安全请求警告
from urllib3 import disable_warnings
from urllib3.exceptions import InsecureRequestWarning
disable_warnings(InsecureRequestWarning)

session = requests.session()
session.verify = False

ua = UserAgent(verify_ssl=False)
headers = {
    "User-Agent": ua.random,
    "Host":"kyfw.12306.cn",
    "Referer":"https://kyfw.12306.cn/otn/passport?redirect=/otn/"
}

def login():
    # 打开登录页面
    url = "https://kyfw.12306.cn/otn/login/init"
    ck = session.get(url, headers=headers)

    # 发送验证码
    if not captcha():
        return False

    # 发送登录信息
    data = {
        "username":USER_NAME,
        "password":PASSWORD,
        "appid":"otn"
    }
    url = "https://kyfw.12306.cn/passport/web/login"
    response = session.post(url, headers=headers, data=data)
    if response.status_code == 200:
        result = json.loads(response.text)
        print(result.get("result_message"), result.get("result_code"))
        if result.get("result_code") != 0:
            return False

    data = {
        "appid":"otn"
    }
    url = "https://kyfw.12306.cn/passport/web/auth/uamtk"
    response = session.post(url, headers=headers, data=data)
    if response.status_code == 200:
        result = json.loads(response.text)
        print(result.get("result_message"))
        newapptk = result.get("newapptk")

    data = {
        "tk":newapptk
    }
    url = "https://kyfw.12306.cn/otn/uamauthclient"
    response = session.post(url, headers=headers, data=data)
    if response.status_code == 200:
        print(response.text)

    url = "https://kyfw.12306.cn/otn/index/initMy12306"
    response = session.get(url, headers=headers)
    if response.status_code == 200 and response.text.find("用户名") != -1:
        return True
    return False

def captcha():
    data = {
        "login_site": "E",
        "module": "login",
        "rand": "sjrand",
        "0.17231872703389062":""
    }

    # 获取验证码
    param = parse.urlencode(data)
    url = "https://kyfw.12306.cn/passport/captcha/captcha-image?{}".format(param)
    response = session.get(url, headers=headers)
    if response.status_code == 200:
        file = BytesIO(response.content)
        img = Image.open(file)
        img.show()

    positions = input("请输入验证码: ")
    # 发送验证码
    data = {
        "answer": positions,
        "login_site": "E",
        "rand": "sjrand"
    }

    url = "https://kyfw.12306.cn/passport/captcha/captcha-check"
    response = session.post(url, headers=headers, data=data)
    if response.status_code == 200:
        result = json.loads(response.text)
        print(result.get("result_message"))
        return True if result.get("result_code") == "4" else False
    return False

if __name__ == "__main__":
    if login():
        print("Success")
    else:
        print("Failed")
