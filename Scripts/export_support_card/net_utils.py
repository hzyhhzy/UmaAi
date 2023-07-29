import requests
import datetime
import time
import os
import json

def get_last_commit_time_from_github(owner, repo, path):
    encoded_path = requests.utils.quote(path, safe='')
    url = f"https://api.github.com/repos/{owner}/{repo}/commits?path={encoded_path}&page=1&per_page=1"
    r = requests.get(url)
    if r.ok:
        date_str = r.json()[0]["commit"]["committer"]["date"]
        return int(time.mktime(datetime.datetime.strptime(date_str, "%Y-%m-%dT%H:%M:%SZ").timetuple()) - time.timezone)
    return -1


def download_file_from_github(owner, repo, branch, path, dst):
    encoded_path = requests.utils.quote(path, safe='')
    url = f"https://raw.githubusercontent.com/{owner}/{repo}/{branch}/{encoded_path}"
    r = requests.get(url)
    if r.ok:
        dirname = os.path.dirname(dst)
        if not os.path.exists(dirname):
            os.makedirs(dirname)
        with open(dst, 'wb') as f:
            f.write(r.content)
        return True
    else:
        print(f"download {url} failed: {r.status_code}")


def get_json_from_github_file(props, local_file, owner, repo, branch, path):
    update = True
    if os.path.exists(local_file):
        update = False
    if update:
        download_file_from_github(owner, repo, branch, path, local_file)
    if os.path.exists(local_file):
        with open(local_file, 'r', encoding='utf-8') as f:
            return json.load(f)
