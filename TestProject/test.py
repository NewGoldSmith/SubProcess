import requests

def get_posts():
    response = requests.get('https://jsonplaceholder.typicode.com/posts')

    if response.status_code == 200:
        return response.json()  # JSONデータをPythonの辞書に変換
    else:
        return None

posts = get_posts()

if posts is not None:
    for post in posts:
        print(post)
else:
    print("API request failed")
    