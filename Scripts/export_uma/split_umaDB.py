import requests
from bs4 import BeautifulSoup
import re
import os
import json

savedir="uma"
try:
    os.mkdir(savedir)
except:
    pass

# 读取JSON文件
with open('umaDB.json', 'r') as file:
    alldata = json.load(file)

for umaid in alldata:
    uma=alldata[umaid]
    filename=str(uma["gameId"])+"-"+uma["name"]
    print(filename)
    filename=savedir+"/"+filename+".json"
    with open(filename, 'w' ,encoding="utf-8") as file:
        s=json.dumps(uma,indent=4)
        file.write(s)
