#pragma once

#include <Arduino.h>

const char *index_html PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Labino</title>
    <style>
        body {font-family: 'Gill Sans', Calibri, sans-serif; }
        .main { margin-left: 1rem; }
        .container { margin-left: 2rem; }
        button { 
            margin: .25rem 0;
            border-radius: 2px;
            border: 1px solid #6e6e6e;
            padding: .2rem .5rem;
            transition-duration: 0.2s;
            cursor: pointer;
            text-decoration: none;
            display: inline;
        }
        button:hover {
            filter: brightness(88%);
        }
        button:active {
            filter: brightness(75%);
        }
        .action { background-color: cadetblue; color: white; }
        .delete { background-color: firebrick; color: white; }
    </style>
    <script>
        function hyperlink(url) {
            window.location.href = url;
        }
    </script>
</head>
<body>
    <div class="main">
        <h1>Home</h1>
        <ul>
            <li><button onclick="hyperlink('/download')">Download log</button></li>
            <li><button onclick="hyperlink('/settings')">Settings</button></li>
        </ul>
    </div>
</body>
</html>)rawliteral";

const char *download_html PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Labino</title>
    <style>
        body {font-family: 'Gill Sans', Calibri, sans-serif; }
        .main { margin-left: 1rem; }
        .container { margin-left: 2rem; }
        button { 
            margin: .25rem 0;
            border-radius: 2px;
            border: 1px solid #6e6e6e;
            padding: .2rem .5rem;
            transition-duration: 0.2s;
            cursor: pointer;
            text-decoration: none;
            display: inline;
        }
        button:hover {
            filter: brightness(88%);
        }
        button:active {
            filter: brightness(75%);
        }
        .action { background-color: cadetblue; color: white; }
        .delete { background-color: firebrick; color: white; }
        .back { font-size: .7rem; }
    </style>
    <script>
        function hyperlink(url) {
            window.location.href = url;
        }
        function deleteConfirm(url) {
            var answer = window.confirm("Delete file?\nThis action can't be undone");
            if (answer)
                hyperlink(url);
        }
    </script>
</head>
<body>
    <div class="main">
        <button class="back" onclick="hyperlink('/index')"><<</button>
        <h1>Download</h1>
        <div class="container">
            <button class="action" onclick="hyperlink('/logfile?preview=1')">Preview log</button><br>
            <button class="action" onclick="hyperlink('/logfile?download=1')">Download log</button><br>
            <button class="delete" onclick="deleteConfirm('/logfile?delete=1')">Delete log</button>
        </div>
    </div>
</body>
</html>)rawliteral";

const char *settings_html PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Labino</title>
    <style>
        body {font-family: 'Gill Sans', Calibri, sans-serif; }
        .main { margin-left: 1rem; }
        .container { margin-left: 2rem; }
        .btn { 
            margin: .25rem 0;
            border-radius: 2px;
            border: 1px solid #6e6e6e;
            padding: .2rem .5rem;
            transition-duration: 0.2s;
            cursor: pointer;
            text-decoration: none;
            display: inline;
        }
        .btn:hover {
            filter: brightness(88%);
        }
        .btn:active {
            filter: brightness(75%);
        }
        .action { background-color: cadetblue; color: white; }
        .delete { background-color: firebrick; color: white; }
        .submit { margin: 1rem 0 }
        .back { font-size: .7rem; }
        form {
            border: 1px solid #6e6e6e;
            border-radius: 3px;
            padding: .5rem 1rem;
            margin-right: 10%;
        }
    </style>
    <script>
        function hyperlink(url) {
            window.location.href = url;
        }
        function getSettings() {
            var xmlhttp = new XMLHttpRequest();

            xmlhttp.onreadystatechange = function() {
                if (xmlhttp.readyState == XMLHttpRequest.DONE) {   // XMLHttpRequest.DONE == 4
                    if (xmlhttp.status == 200) {
                        console.log(xmlhttp.responseText);
                        obj = JSON.parse(xmlhttp.responseText);
                        document.getElementById("dbxUpload").checked = obj.dbxUpload;
                        document.getElementById("internetStatus").checked = obj.internet;
                    }
                    else if (xmlhttp.status == 400) {
                        console.log('There was an error 400');
                    }
                    else {
                        console.log('something else other than 200 was returned');
                    }
                }
            };

            xmlhttp.open("GET", "/getsettings", true);
            xmlhttp.send();
        }
    </script>
</head>
<body>
    <script>
        getSettings();
    </script>
    <div class="main">
        <button class="back" onclick="hyperlink('/index')"><<</button>
        <br><br>
        <div class="container">
            <form action="/setsettings" method="GET">
                <h1>Settings</h1>
                <input type="hidden" name="dbxUpload" value="0">
                <label><input type="checkbox" id="dbxUpload" value="1", name="dbxUpload">Upload to dropbox</label><br>
                <input class="btn submit" type="submit" value="Set">
            </form>
            <br>
            <form action="">
                <h1>Info</h1>
                <label><input type="checkbox" id="internetStatus" onclick="return false;">Internet connection</label><br>
            </form>
        </div>
    </div>
</body>
</html>)rawliteral";
