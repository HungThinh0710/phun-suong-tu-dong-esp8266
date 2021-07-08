#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <StreamUtils.h>
#include "LittleFS.h"
#include <NTPtimeESP.h>

int output_pin = 16; // ~ GPIO16 (D0)

/** GLOBAL VARIABLES **/
String scheduleTextRaw;
int isActivatedSchedule = 0; //Status activate schedule (from LittleFS file)
boolean isExecuteAble = false; //Status execute D0 PIN
int minuteExecutePin = 0; //How many minute will be execute D0 PIN.

//Initial HTTPServer   |Port|
ESP8266WebServer  server(80);

//Initial NTP Time Server
NTPtime NTPch("vn.pool.ntp.org");   // Server NTP
strDateTime dateTime;

//Initial time run execute ms
unsigned long timeTmpSave = 0;

void returnPage() {
  String html = prepareHTML();
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/html", html);
}

void refreshSchedules() {
  File testFile = LittleFS.open(F("/data.txt"), "r");
  if (testFile) {
    String text = testFile.readString();
    scheduleTextRaw = text;
  }
}

void activeSchedule() {
  File scheduleActivateStatusFile = LittleFS.open(F("/activate_daily.txt"), "w");
  if (scheduleActivateStatusFile) {
    scheduleActivateStatusFile.print("1");
    isActivatedSchedule = 1;
    scheduleActivateStatusFile.close();
  }
}

void deactivateSchedule() {
  File scheduleActivateStatusFile = LittleFS.open(F("/activate_daily.txt"), "w");
  if (scheduleActivateStatusFile) {
    scheduleActivateStatusFile.print("0");
    isActivatedSchedule = 0;
    scheduleActivateStatusFile.close();
  }
}
/**
   Save Schedules
*/
void saveSchedule() {
  String data;
  data = server.arg("data");
  if (data.length() == 0) {
    data = "";
  }
  File dataFile = LittleFS.open(F("/data.txt"), "w");
  if (dataFile) {
    dataFile.print(data);
    dataFile.close();
  }
  refreshSchedules();
}

String prepareHTML() {
  int statusPhunSuong = digitalRead(output_pin);
  Serial.print("Status_PS: ");
  Serial.println(statusPhunSuong);

  //scheduleTextRaw
  //isActivatedSchedule
  //statusPhunSuong
  //  html += " <input id=\"data_server\" value=\""+scheduleTextRaw+"\" /> <input id=\"data_schedule_status\" value=\""+isActivatedSchedule+"\" /> <input id=\"phunsuong_status\" value=\""+statusPhunSuong+"\" /> ";

  //Convert HTML to Arduino String return here: http://davidjwatts.com/youtube/esp8266/esp-convertHTM.html#
  String html = "<!DOCTYPE html> <html> <head> <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> <title>Hệ thống phun sương</title> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <link rel=\"icon\" href=\"data:,\"> <style> /* for xl */ .custom-switch.custom-switch-xl .custom-control-label { padding-left: 4rem; padding-bottom: 2.5rem; } .custom-switch.custom-switch-xl .custom-control-label::before { height: 2.5rem; width: calc(4rem + 0.75rem); border-radius: 5rem; } .custom-switch.custom-switch-xl .custom-control-label::after { width: calc(2.5rem - 4px); height: calc(2.5rem - 4px); border-radius: calc(4rem - (2.5rem / 2)); } .custom-switch.custom-switch-xl .custom-control-input:checked~.custom-control-label::after { transform: translateX(calc(2.5rem - 0.25rem)); } </style> <link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.6.0/dist/css/bootstrap.min.css\" integrity=\"sha384-B0vP5xmATw1+K9KRQjQERJvTumQW0nPEzvF6L/Z6nronJ3oUOFUFpCjEUQouq2+l\" crossorigin=\"anonymous\"> </head> <body> <div style=\"display:none\"> ";
  html += " <input id=\"data_server\" value=\"" + scheduleTextRaw + "\" /> <input id=\"data_schedule_status\" value=\"" + isActivatedSchedule + "\" /> <input id=\"phunsuong_status\" value=\"" + statusPhunSuong + "\" /> ";
  html += " </div> <div class=\"container-fluid mt-2\"> <h2 class=\"text-center\">Hệ Thống Phun Sương</h2> <div class=\"card\"> <div class=\"card-body\"> <h5 class=\"card-title\">Tuỳ chỉnh phun sương bằng tay</h5> <h6 id=\"wrapper-badge-phunsuong-div\" class=\"card-subtitle mb-2 text-muted\"> </h6> <div id=\"phunsuong_content_div\" class=\"d-flex justify-content-center\"></div> </div> </div> <div class=\"card mt-3\"> <div class=\"card-body\"> <h5 class=\"card-title\">Tuỳ chỉnh phun sương tự động</h5> <h6 id=\"status_schedule\" class=\"card-subtitle mb-2 text-muted\"></h6> <div class=\"custom-control custom-switch custom-switch-xl text-center\"> <input type=\"checkbox\" class=\"custom-control-input\" id=\"toogleAutoSchedule\"> <label class=\"custom-control-label\" for=\"toogleAutoSchedule\"></label> </div> <div class=\"mt-4 text-center\"> <form method=\"GET\" action=\"#\"> <p class=\"\"><b>Chỉnh thời gian tự động</b></p> <div class=\"row mt-2\"> <div class=\"col\"> <div class=\"form-group\"> <label>Thời gian</label> <div class=\"d-flex flex-row bd-highlight mb-3\"> <input id=\"time\" type=\"time\" name=\"time\" value=\"\" class=\"ml-2 form-control\" required /> </div> </div> </div> <div class=\"col\"> <div class=\"form-group\"> <label>TG phun (phút)</label> <input id=\"during\" type=\"number\" name=\"range\" value=\"\" class=\"form-control\" required> </div> </div> </div> <h5 class=\"text-left\">Chức năng</h5> <div class=\"text-left card p-2\"> <button type=\"button\" id=\"save\" class=\"btn btn-success mb-2\">Lưu</button> <a href=\"/clear-all-schedule\" class=\"btn btn-danger mb-2\">Xoá tất cả lịch đã lưu</a> <button class=\"btn btn-primary\" type=\"button\" data-toggle=\"collapse\" data-target=\"#collapseListSavedSchedule\" aria-expanded=\"false\" aria-controls=\"collapseListSavedSchedule\"> Xem danh sách thời gian biểu </button> </div> </form> <div class=\"collapse\" id=\"collapseListSavedSchedule\"> <div class=\"mt-5\"> <h4 class=\"text-center mt-2\">Danh sách thời gian biểu</h4> <!-- Tab panes --> <div class=\"tab-content\"> <div id=\"all\" class=\"tab-pane active\"><br> <div id=\"content-daily-schedule\" class=\"text-left\"> </div> </div> </div> </div> </div> </div> </div> </div> <footer class=\"text-mute mt-5\"> <div class=\"container text-center\"> <p>Phun sương không còn chạy bằng cơm</p> <p>Copyright © 2021 - By Hưng Thịnh</p> </div> </footer> <script> function getDataRawFromServer() { let dataRaw = document.getElementById(\"data_server\").value; let data = dataRaw.substring(0, dataRaw.length - 1).trim(); return data; } function additionDataToSendSever(raw, addition) { console.log(raw.length); let data = raw; if (data.length > 0) { data += \"-\"; } data += addition + \"-\"; return data; } function onToggleChange() { let isChecked = document.getElementById(\"toogleAutoSchedule\").checked; if (!isChecked) { setTimeout(function () { document.location.href = \"/deactivate-schedule\"; }, 450); } else { setTimeout(function () { document.location.href = \"/activate-schedule\"; }, 450); } } function initMistStatus() { let statusSchedule = document.getElementById(\"phunsuong_status\").value; let badgePhunSuongDiv = document.getElementById(\"wrapper-badge-phunsuong-div\"); let phunSuongContentDiv = document.getElementById(\"phunsuong_content_div\"); if (statusSchedule == 1) { badgePhunSuongDiv.innerHTML += `Trạng thái: <span class=\"badge badge-info\">Đang hoạt động</span>`; phunSuongContentDiv.innerHTML += `<a href=\"/off\" class=\"btn btn-danger\">Tắt </a>`; } else { badgePhunSuongDiv.innerHTML += `Trạng thái: <span class=\"badge badge-danger\">Đang tắt</span>`; phunSuongContentDiv.innerHTML += `<a href=\"/on\" class=\"btn btn-success\">Kích hoạt phun sương</a>`; } } function initAndHandleData() { initMistStatus(); let statusSchedule = document.getElementById(\"data_schedule_status\"); let statusScheduleDiv = document.getElementById(\"status_schedule\"); if (statusSchedule.value == 0) { console.log(\"Deactivate Status\"); statusScheduleDiv.innerHTML += `Trạng thái: <span class=\"badge badge-danger\">Đã tắt</span>`; document.getElementById(\"toogleAutoSchedule\").checked = !1; } else { statusScheduleDiv.innerHTML += `Trạng thái: <span class=\"badge badge-primary\">Đang kích hoạt</span>`; document.getElementById(\"toogleAutoSchedule\").checked = 1; } let timeWillExecute = [], timeDuringExecute = []; let div = document.getElementById(\"content-daily-schedule\"); let data = getDataRawFromServer(); if (data.length <= 0) { div.innerHTML += `<p>Không có dữ liệu nào</p>`; } else { let dataDedicatedArray = data.split(\"-\"); let totalSchedules = dataDedicatedArray.length; dataDedicatedArray.map(item => { let tmp = item.split(\"|\"); timeWillExecute.push(tmp[0]); timeDuringExecute.push(tmp[1]); }); if (totalSchedules > 0) { for (let i = 0; i < totalSchedules; i++) { div.innerHTML += `<p class=\"schedules-time\" data-time=\"${timeWillExecute[i]}\">Thời gian: <b>${timeWillExecute[i]}</b> - Chạy <b>${timeDuringExecute[i]}</b> phút - ❌</p>`; } } } document.getElementById(\"save\").onclick = function (e) { const endpoint = \"/save-schedule\"; let finalData = null; var time = document.getElementById('time').value; var during = document.getElementById('during').value; if (time.length < 1 || during.length < 1) { alert(\"Vui lòng nhập thời gian và TG phun\"); return; } let hourAndMinutesArr = time.split(\":\"); let hour = hourAndMinutesArr[0]; let minute = parseInt(hourAndMinutesArr[1]); let hourAndMinuteText = `${hour}:${minute}`; const isFounded = timeWillExecute.find(element => element == hourAndMinuteText); let iTimeAr = null; if (typeof (isFounded) != 'undefined') { const isLargeNumber = (element) => element == hourAndMinuteText; let indexDuring = timeWillExecute.findIndex(isLargeNumber); console.log(timeWillExecute.findIndex(isLargeNumber)); finalData = data.replace(`${hourAndMinuteText}|${timeDuringExecute[indexDuring]}`, `${hourAndMinuteText}|${during}`); if (finalData.length > 0) { finalData += \"-\"; } } else { console.log(\"NOT THING\"); let prepareDataSend = `${hour}:${minute}|${during}`; finalData = additionDataToSendSever(data, prepareDataSend); } window.location.href = `${endpoint}?data=${finalData}`; }; var savedSchedulesElements = document.getElementsByClassName(\"schedules-time\"); Array.from(savedSchedulesElements).forEach(v => v.addEventListener('click', function () { const endpoint = \"/save-schedule\"; let timeNeedToDelete = this.dataset.time; let hourAndMinutesArr = timeNeedToDelete.split(\":\"); let hour = hourAndMinutesArr[0]; let minute = parseInt(hourAndMinutesArr[1]); let hourAndMinuteText = `${hour}:${minute}`; const isLargeNumber = (element) => element == hourAndMinuteText; let indexDuring = timeWillExecute.findIndex(isLargeNumber); data += \"-\"; finalData = data.replace(`${hourAndMinuteText}|${timeDuringExecute[indexDuring]}-`, ``); window.location.href = `${endpoint}?data=${finalData}`; })); } document.getElementById(\"toogleAutoSchedule\").addEventListener(\"change\", onToggleChange); initAndHandleData(); </script> <script src=\"https://code.jquery.com/jquery-3.5.1.slim.min.js\" integrity=\"sha384-DfXdz2htPH0lsSSs5nCTpuj/zy4C+OGpamoFVy38MVBnE+IbbVYUew+OrCXaRkfj\" crossorigin=\"anonymous\"></script> <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@4.6.0/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-Piv4xVNRyMGpqkS2by6br4gNJ7DXjqk09RmUpJ8jgGtD7zP9yug3goQfGII0yAns\" crossorigin=\"anonymous\"></script> </body> </html>";
  return html;
}

void handleRoot() {
  String html = prepareHTML();
  server.send(200, "text/html", html);
}

void clearData() {
  File dataFile = LittleFS.open(F("/data.txt"), "w");

  if (dataFile) {
    Serial.println("CLEAR_CONTENT_DATA!");
    dataFile.print("");
    dataFile.close();
  } else {
    Serial.println("Problem on clear content in data.txtfile!");
  }

  refreshSchedules();
}

void setup() {

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(output_pin, OUTPUT);
  digitalWrite(output_pin, 0);

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  //reset settings - wipe credentials for testing
  //wm.resetSettings();

  bool res;
  res = wm.autoConnect("HeThongPhunSuong"); // anonymous ap

  if (!res) {
    Serial.println("Failed to connect");
    //ESP.restart();
  }
  else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    if (LittleFS.begin()) {
      Serial.println(F("done."));
    } else {
      Serial.println(F("fail."));
    }

    // To format all space in LittleFS
    // LittleFS.format()

    Serial.println();

    /** Read content (data.txt) & (activate_daily.txt) **/
    File dataFile = LittleFS.open(F("/data.txt"), "r");
    File activate_daily = LittleFS.open(F("/activate_daily.txt"), "r");
    if (dataFile) {
      String text = dataFile.readString();
      scheduleTextRaw = text; // clone to global variable
      dataFile.close();
    } else {
      Serial.println("Problem on read data.txt file!");
    }

    if (activate_daily) {
      String result = activate_daily.readString();
      isActivatedSchedule = result.toInt();
    }
    else {
      Serial.println("Problem on read daily_activate.txt file!");
    }


    /*** ROUTE WEBSERVER***/
    server.on("/", handleRoot);

    server.on("/on", []() {
      if (digitalRead(output_pin) == 0)
        digitalWrite(output_pin, 1);
      returnPage();
    });

    server.on("/off", []() {
      if (digitalRead(output_pin) == 1)
        digitalWrite(output_pin, 0);
      returnPage();
    });

    server.on("/clear-all-schedule", []() {
      clearData();
      returnPage();
    });

    server.on("/save-schedule", []() {
      saveSchedule();
      returnPage();
    });

    server.on("/activate-schedule", []() {
      activeSchedule();
      returnPage();
    });

    server.on("/deactivate-schedule", []() {
      deactivateSchedule();
      returnPage();
    });

    server.onNotFound(returnPage);

    server.begin();

    Serial.println("HTTP server started");

    if (!MDNS.begin("phunsuong")) {  // Khởi tạo mDNS cho phunsuong.local
      Serial.println("Error setting up MDNS responder!");
    }

    // Broadcasting The Type Of Service The ESP8266 Provides Through mDNS
    MDNS.addService("http", "tcp", 80);

  }

}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  MDNS.update();
  if (isActivatedSchedule == 1 ) {
    dateTime = NTPch.getNTPtime(7.0, 0);
    if (dateTime.valid) {

      //    NTPch.printDateTime(dateTime);

      /*******  GET CURRENT SCHEDULE    *******/
      int count = 0;
      String textRaw = scheduleTextRaw;
      //Counts each character except space
      for (int i = 0; i < textRaw.length(); i++) {
        if (textRaw[i] == '-')
          count++;
      }

      String listScheduleArr[count];
      int r = 0, t = 0;

      for (int i = 0; i < textRaw.length(); i++) {
        if (textRaw.charAt(i) == '-') {
          listScheduleArr[t] = textRaw.substring(r, i);
          r = (i + 1);
          t++;
        }
      }

      String timeWillBeExecute[count];
      int timeExecute[count];

      //  size_t sizeListSchedule = sizeof(a)/sizeof(a[0]);
      int rTmp = 0, tTmp = 0;
      for (int i = 0; i < count; i++) {
        for (int j = 0; j < listScheduleArr[i].length(); j++) {
          //       Serial.println(listScheduleArr[j]);
          if (listScheduleArr[i].charAt(j) == '|') {
            //          Serial.println("We founded it.");

            String s1 = listScheduleArr[i].substring(rTmp, j);
            int s2 = listScheduleArr[i].substring(j + 1, listScheduleArr[i].length()).toInt();
            timeWillBeExecute[i] = s1;
            timeExecute[i] = s2;
            //
            //          Serial.print("Time: ");
            //          Serial.println(s1);
            //
            //          Serial.print("During: ");
            //          Serial.println(s2);
          }
        }
      }

      String currentTime = (String) dateTime.hour + ":" + (String) dateTime.minute;

      if (currentTime == "23:00" ) { //RESET DAILY
        ESP.restart();
        return;
      }

      for (int i = 0; i < count; i++) {
        if (timeWillBeExecute[i] == currentTime) {
          //      if (currentTime == "15:55" || currentTime == "15:58" || currentTime == "16:1" ) { //DEBUG ONLY
          Serial.println("Inside Current Time");
          if (!isExecuteAble) {
            timeTmpSave = millis();
            minuteExecutePin = timeExecute[i];
            isExecuteAble = true;
          }
        }
      }
    }

    int finalTimeExecute = minuteExecutePin * 60000;
    //31000    -    0         <    20000 + 10000;
    //  Serial.println("==================================");
    //  Serial.print(millis());
    //  Serial.print(" < ");
    //  Serial.print(timeTmpSave);
    //  Serial.print(" +  ");
    //  Serial.print(finalTimeExecute);
    //  Serial.print("(");
    //  Serial.print(timeTmpSave + finalTimeExecute);
    //  Serial.println(")");
    //  Serial.println("==================================");

    if (isExecuteAble) {
      if ( millis() <  timeTmpSave + finalTimeExecute) {
        //      timeTmpSave = millis();
        if (digitalRead(output_pin) == 0) {
          Serial.println("Virtual TURN LIGHT ON___===>");
          digitalWrite(output_pin, 1);
        }
      }
      else {
        isExecuteAble = false;
        minuteExecutePin = 0;
        if (digitalRead(output_pin) == 1) {
          Serial.println("Virtual TURN LIGHT OFF");
          digitalWrite(output_pin, 0);
        }
      }
    }
  }

}
