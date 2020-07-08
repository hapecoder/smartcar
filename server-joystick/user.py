#user.py
import os

if not os.path.exists('./userinfo.txt'):
    open('./userinfo.txt', 'w', encoding = 'utf-8').close()
    
def read_user(username, password):
    userinfo = dict()
    with open('./userinfo.txt', 'r') as fob:
        for line in fob.readlines():
            uname = line.strip().split('=>')[0]
            try:
                pwd = line.strip().split('=>')[1]
                userinfo[uname] = pwd
            except:
                print('\033[1;31;40m  严重:用户信息文件格式错误,系统无法运行 \033[0m')
                exit(1)
    if username not in userinfo:
        return False
    if userinfo[username] == password:
        return True
    return False