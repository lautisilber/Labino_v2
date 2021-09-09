from os import makedirs
import dropbox
import json
from os.path import isfile, dirname, isdir, basename, realpath, join

logs_folder = '/logs'
config_file = 'config.json'

config = None
if isfile(config_file):
    try:
        with open(config_file, 'r') as file:
            config = json.load(file)
    except:
        print('Couldn\'t load config')


def get_dbx(token=None):
    global config
    if token is None and config is not None:
        if 'dbx_token' in config:
            token = config['dbx_token']
    return dropbox.Dropbox(token)


def download_file(remote_path: str, local_path: str, dbx=None):
    if dbx is None:
        dbx = get_dbx()
    if not isdir(dirname(local_path)):
        makedirs(dirname(local_path))
    with open(local_path, "wb") as f:
        metadata, res = dbx.files_download(path=remote_path)
        f.write(res.content)
        print(f'downloaded \'{local_path}\'')


def get_all_files_in_folder(remote_folder: str=logs_folder, dbx=None) -> list:
    res = dbx.files_list_folder(remote_folder)
    files = [e.path_lower for e in res.entries]
    return files


def download_folder(remote_folder: str=logs_folder, local_folder=None, dbx=None):
    if dbx is None:
        dbx = get_dbx()
    cwd = dirname(realpath(__file__))
    if local_folder is not None:
        local_folder = join(cwd, local_folder)
    else:
        local_folder = remote_folder[1:] if remote_folder[0] == '/' else remote_folder
        local_folder = join(cwd, local_folder)
    if not isdir(local_folder):
        makedirs(local_folder)
    files = get_all_files_in_folder(remote_folder, dbx)
    for file in files:
        local_file = join(local_folder, basename(file))
        download_file(file, local_file, dbx)

    
def delete_folder(remote_folder: str=logs_folder, dbx=None):
    if dbx is None:
        dbx = get_dbx()
    files = get_all_files_in_folder(remote_folder, dbx)
    for file in files:
        dbx.files_delete(file)
        print(f'deleted remote {file}')


'''def download_zip(remote_folder: str='/logs', local_folder=None, dbx=None):
    if dbx is None:
        dbx = get_dbx()
    cwd = dirname(realpath(__file__))
    print(cwd)
    if local_folder is not None:
        print('AAA')
        local_folder = join(cwd, local_folder)
    else:
        print('BBB')
        local_folder = remote_folder[1:] if remote_folder[0] == '/' else remote_folder
        local_folder = join(cwd, local_folder)
    print(local_folder)
    if not isdir(local_folder):
        makedirs(local_folder)
    dbx.files_download_zip_to_file(remote_folder, local_folder)'''


def change_config(**kwargs):
    if not isfile(config_file):
        return
    try:
        with open(config_file, 'r') as file:
            config = json.load(file)
        for key in kwargs:
            config[key] = kwargs[key]
        with open(config_file, 'w') as file:
            json.dump(config, file)
    except:
        print('Couldn\'t change config')

    
def change_dbx_token(token: str):
    change_dbx_token(dbx_token=token)
