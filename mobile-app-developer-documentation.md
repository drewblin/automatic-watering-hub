# Документація для розробника мобільного додатку

Цей документ описує протокол взаємодії мобільного додатку з контролером
automatic-watering-hub через BLE і Wi-Fi HTTPS API, а також взаємодію
мобільного додатку з backend-сервером для push-логів і графіків метрик.

## Загальна логіка взаємодії

1. Мобільний додаток знаходить BLE-пристрій `Automatic Watering Hub`.
2. Додаток підключається до BLE service
   `4d42b2d0-35ba-4b70-b8a2-d1cf01e904c1`.
3. Під час першого підключення виконується зашифрований authenticated pairing із
   6-значним passkey `482917`.
4. Через BLE додаток:
   - читає або записує Wi-Fi settings;
   - читає IP-адресу контролера;
   - читає `apiAccessToken` для HTTPS API;
   - може підписатися на BLE log notifications.
5. Після збереження Wi-Fi settings контролер відповідає
   `restartScheduled: true` і перезапускається.
6. Коли контролер підключився до Wi-Fi, додаток читає IP через BLE або повторно
   підключається до BLE після reboot.
7. Для основного керування додаток використовує HTTPS API:
   `https://<controller-ip>/api/...`
8. Кожен HTTPS-запит повинен містити заголовок:
   `Authorization: Bearer <apiAccessToken>`.
9. Для історичних логів, push-сповіщень і графіків метрик додаток
   використовує server API. Контролер сам відправляє логи і метрики на сервер,
   якщо в `remoteLogSettings` заданий URL.

Wi-Fi settings не повертаються і не змінюються через HTTPS API. Це навмисно
залишено тільки в BLE-каналі.

## Єдиний формат відповідей

BLE read/write results і HTTPS responses повертають JSON однакової форми:

```json
{
  "success": true,
  "data": {},
  "error": null
}
```

Якщо сталася помилка:

```json
{
  "success": false,
  "data": {},
  "error": "Missing field: ssid"
}
```

Для HTTPS додатково використовується HTTP status code. Поточні статуси:

- `200 OK` - успішна операція.
- `400 Bad Request` - невалідний JSON або невалідні поля.
- `401 Unauthorized` - неправильний або відсутній `Authorization`.
- `404 Not Found` - ресурс не знайдено, наприклад valve pin.
- `413 Content Too Large` - тіло запиту більше за 16 KiB.
- `500 Internal Server Error` - помилка збереження або внутрішня помилка.
- `503 Service Unavailable` - water hub недоступний.

## BLE

Назва пристрою: `Automatic Watering Hub`.

Захист:

- зашифрований authenticated pairing;
- passkey: `482917`;
- bond-ключі зберігаються, тому відомому телефону не потрібно повторювати
  pairing під час наступних підключень.

Advertising:

- перші 5 хвилин після старту fast advertising;
- після цього slow advertising;
- після disconnect advertising запускається повторно.

UUID сервісу:

```text
4d42b2d0-35ba-4b70-b8a2-d1cf01e904c1
```

### BLE characteristics

| Назва | UUID | Властивості | Опис |
| --- | --- | --- | --- |
| WifiSettings | `4d42b2d1-35ba-4b70-b8a2-d1cf01e904c1` | `READ`, encrypted, authenticated | Поточні Wi-Fi settings |
| SaveWifiSettings | `4d42b2d2-35ba-4b70-b8a2-d1cf01e904c1` | `READ`, `WRITE`, encrypted, authenticated | Запис нових Wi-Fi settings; результат читається з цього ж characteristic value |
| WifiIpAddress | `4d42b2d3-35ba-4b70-b8a2-d1cf01e904c1` | `READ`, encrypted, authenticated | Поточна IP-адреса Wi-Fi інтерфейсу і mDNS hostname |
| ApiAccessToken | `4d42b2d4-35ba-4b70-b8a2-d1cf01e904c1` | `READ`, encrypted, authenticated | Bearer token для HTTPS API |
| LogNotifications | `4d42b2d5-35ba-4b70-b8a2-d1cf01e904c1` | `READ`, `NOTIFY`, encrypted, authenticated | BLE log notifications |

### Читання Wi-Fi settings

Прочитати characteristic:
`4d42b2d1-35ba-4b70-b8a2-d1cf01e904c1`.

Відповідь:

```json
{
  "success": true,
  "data": {
    "wifiSettings": {
      "ssid": "network-name",
      "password": "network-password"
    }
  },
  "error": null
}
```

### Збереження Wi-Fi settings

Записати JSON у characteristic:
`4d42b2d2-35ba-4b70-b8a2-d1cf01e904c1`.

Запит:

```json
{
  "ssid": "network-name",
  "password": "network-password"
}
```

Значення відповіді в тому самому characteristic:

```json
{
  "success": true,
  "data": {
    "restartScheduled": true
  },
  "error": null
}
```

Після успішного запису контролер планує restart. Додаток має показати стан
очікування, від'єднатися від BLE, зачекати reboot і повторно знайти пристрій.

### Читання Wi-Fi IP address і hostname

Прочитати characteristic:
`4d42b2d3-35ba-4b70-b8a2-d1cf01e904c1`.

Відповідь:

```json
{
  "success": true,
  "data": {
    "ipAddress": "192.168.1.42",
    "hostname": "watering-hub-a1b2c3",
    "localHostname": "watering-hub-a1b2c3.local"
  },
  "error": null
}
```

Якщо Wi-Fi ще не підключений, значення може бути `0.0.0.0`. Додаток повинен
повторювати читання з паузою або просити користувача перевірити Wi-Fi settings.
`hostname` генерується контролером з MAC-адреси і є стабільним для конкретного
пристрою. `localHostname` можна використовувати для HTTPS API в локальній мережі,
якщо mDNS підтримується мережею і мобільною платформою. Якщо mDNS не резолвиться,
додаток має fallback на `ipAddress`.

### Читання API access token

Прочитати characteristic:
`4d42b2d4-35ba-4b70-b8a2-d1cf01e904c1`.

Відповідь:

```json
{
  "success": true,
  "data": {
    "apiAccessToken": "64-hex-character-token"
  },
  "error": null
}
```

Token генерується контролером під час першого запуску і зберігається в NVS.
Додаток має зберігати його як секрет.

### BLE log notifications

Підписатися на notifications у characteristic:
`4d42b2d5-35ba-4b70-b8a2-d1cf01e904c1`.

Notification payload:

```json
{
  "level": "info",
  "uptimeMs": 123456,
  "tag": "ApiServerWifi",
  "message": "message text"
}
```

Можливі `level`: `error`, `warning`, `info`, `debug`, `verbose`.

## Wi-Fi HTTPS API

Базовий URL:

```text
https://<controller-ip>
```

Порт за замовчуванням: `443`.

Кожен запит повинен містити заголовки:

```http
Authorization: Bearer <apiAccessToken>
Content-Type: application/json
```

`Content-Type` потрібен для запитів із body. GET-запити body не мають.

HTTPS використовує самопідписаний certificate. Мобільний додаток повинен або
пінити сертифікат, або перевіряти SHA-256 fingerprint:

```text
DE:B7:7B:DC:88:1B:09:EE:23:19:8D:72:06:FA:E6:AD:F9:E4:8A:F1:5B:1D:EE:BB:4F:58:7F:0E:2F:42:B3:AC
```

### GET /api/settings

Повертає поточні налаштування контролера і поточний час контролера.

Запит:

```http
GET /api/settings
Authorization: Bearer <apiAccessToken>
```

Відповідь:

```json
{
  "success": true,
  "data": {
    "settings": {
      "globalSettings": {
        "idleWaterCounterReadIntervalSeconds": 300,
        "wateringWaterCounterReadIntervalSeconds": 60,
        "idlePressureSensorReadIntervalSeconds": 300,
        "wateringPressureSensorReadIntervalSeconds": 5,
        "idleSoilSensorReadIntervalSeconds": 3600,
        "wateringSoilSensorReadIntervalSeconds": 60,
        "maximumManualValveOpenTimeSeconds": 3600,
        "startWateringBelowHumidityPercent": 20,
        "stopWateringAboveHumidityPercent": 80,
        "wateringStartMode": "immediately",
        "wateringWindowStartTime": null,
        "wateringWindowEndTime": null,
        "zoneWateringDurationSeconds": 300,
        "zoneWateringRetryDelaySeconds": 900
      },
      "remoteLogSettings": {
        "url": "https://api.example.com/controller-ingest/devices/dev_123",
        "token": "controller-ingest-token"
      },
      "valveSettings": [
        {
          "pin": 19,
          "name": "Zone 1",
          "soilSensorSlaveAddress": 1
        }
      ],
      "pressureSensor": {
        "slaveAddress": 2,
        "name": "Pressure sensor"
      },
      "magistralWaterCounterSetting": {
        "pin": 36,
        "name": "Main water counter",
        "litersPerTick": 1.0
      },
      "leafWaterCounterSettings": [
        {
          "pin": 34,
          "name": "Column 1",
          "litersPerTick": 1.0
        }
      ],
      "soilSensorSettings": [
        {
          "slaveAddress": 1,
          "name": "Soil sensor 1"
        }
      ]
    },
    "controllerCurrentTimestamp": 1717245600,
    "controllerCurrentTime": "2024-06-01T12:00:00+0300"
  },
  "error": null
}
```

`controllerCurrentTimestamp` і `controllerCurrentTime` можуть бути `null`, якщо
час ще не синхронізований.

### PUT /api/settings

Зберігає налаштування контролера. Після успішного збереження контролер
відповідає і перезапускається.

Запит:

```http
PUT /api/settings
Authorization: Bearer <apiAccessToken>
Content-Type: application/json
```

Тіло запиту:

```json
{
  "globalSettings": {
    "idleWaterCounterReadIntervalSeconds": 300,
    "wateringWaterCounterReadIntervalSeconds": 60,
    "idlePressureSensorReadIntervalSeconds": 300,
    "wateringPressureSensorReadIntervalSeconds": 5,
    "idleSoilSensorReadIntervalSeconds": 3600,
    "wateringSoilSensorReadIntervalSeconds": 60,
    "maximumManualValveOpenTimeSeconds": 3600,
    "startWateringBelowHumidityPercent": 20,
    "stopWateringAboveHumidityPercent": 80,
    "wateringStartMode": "withinWateringWindow",
    "wateringWindowStartTime": {
      "hour": 23,
      "minute": 10
    },
    "wateringWindowEndTime": {
      "hour": 6,
      "minute": 30
    },
    "zoneWateringDurationSeconds": 300,
    "zoneWateringRetryDelaySeconds": 900
  },
  "remoteLogSettings": {
    "url": "https://api.example.com/controller-ingest/devices/dev_123",
    "token": "controller-ingest-token"
  },
  "valveSettings": [
    {
      "pin": 19,
      "name": "Zone 1",
      "soilSensorSlaveAddress": 1
    }
  ],
  "pressureSensor": {
    "slaveAddress": 2,
    "name": "Pressure sensor"
  },
  "magistralWaterCounterSetting": {
    "pin": 36,
    "name": "Main water counter",
    "litersPerTick": 1.0
  },
  "leafWaterCounterSettings": [
    {
      "pin": 34,
      "name": "Column 1",
      "litersPerTick": 1.0
    }
  ],
  "soilSensorSettings": [
    {
      "slaveAddress": 1,
      "name": "Soil sensor 1"
    }
  ]
}
```

Відповідь:

```json
{
  "success": true,
  "data": {
    "restartScheduled": true
  },
  "error": null
}
```

Правила валідації:

- максимальний розмір тіла запиту: 16 KiB;
- arrays `valveSettings`, `leafWaterCounterSettings`, `soilSensorSettings`:
  максимум 32 items кожен;
- інтервали часу мають бути від `1` до `2147483` seconds;
- пороги вологості мають бути `0..100`, і
  `startWateringBelowHumidityPercent < stopWateringAboveHumidityPercent`;
- `wateringStartMode` має бути `immediately` або `withinWateringWindow`;
- коли `wateringStartMode` дорівнює `withinWateringWindow`,
  `wateringWindowStartTime.hour`, `wateringWindowEndTime.hour` мають бути
  `0..23`, а `minute` в обох об'єктах має бути `0..59`;
- якщо `wateringWindowEndTime` менший за `wateringWindowStartTime`, вікно
  поливу переходить через північ;
- у режимі `withinWateringWindow` контролер запускає і продовжує автоматичний
  полив тільки всередині вікна; коли вікно завершується, активний автоматичний
  полив закривається;
- `pressureSensor` може бути `null` або object;
- `magistralWaterCounterSetting` може бути `null` або object;
- Modbus slave addresses мають бути `1..247`;
- slave addresses датчиків вологості мають бути унікальними;
- address датчика тиску має відрізнятися від soil sensor addresses;
- кожен valve має посилатися на наявний soil sensor через
  `soilSensorSlaveAddress`;
- GPIO pins мають бути унікальними для клапанів, магістрального лічильника води
  і додаткових лічильників води;
- `litersPerTick` має бути додатним скінченним числом.

`remoteLogSettings.url` - базовий server URL, на який контролер відправляє
outbound telemetry. Firmware додає до нього `/logs` і `/metrics`.
`remoteLogSettings.token` - bearer token тільки для Controller -> Server ingest.
Мобільний додаток не повинен використовувати цей token як session token користувача.

Для `wateringStartMode: "immediately"` треба відправляти
`"wateringWindowStartTime": null` і `"wateringWindowEndTime": null` або не
показувати ці значення в UI. Поточний parser firmware вимагає обидва поля
тільки коли mode дорівнює `withinWateringWindow`.

Важливо для UI: автоматичний полив і water-hub routes запускаються тільки після
повного налаштування базової системи. Треба задати `pressureSensor`,
`magistralWaterCounterSetting`, мінімум один `soilSensorSettings` item і клапани,
які посилаються на наявні soil sensors. Якщо ці налаштування неповні, контролер
зберігає конфігурацію, але полив не стартує, а endpoints керування клапанами і
сенсорними метриками можуть бути недоступні.

### GET /api/sensors/metrics

Повертає останні відомі значення сенсорів.

Запит:

```http
GET /api/sensors/metrics
Authorization: Bearer <apiAccessToken>
```

Відповідь:

```json
{
  "success": true,
  "data": {
    "sensors": [
      {
        "sensorId": 2,
        "sensorType": "pressure",
        "name": "Pressure sensor",
        "value": 2.4,
        "uptimeMs": 123456
      },
      {
        "sensorId": 36,
        "sensorType": "water_counter",
        "name": "Main water counter",
        "value": 15.5,
        "uptimeMs": 123456
      },
      {
        "sensorId": 1,
        "sensorType": "soil_temperature",
        "name": "Soil sensor 1",
        "value": 21.8,
        "uptimeMs": 123456
      },
      {
        "sensorId": 1,
        "sensorType": "soil_humidity",
        "name": "Soil sensor 1",
        "value": 64.2,
        "uptimeMs": 123456
      }
    ]
  },
  "error": null
}
```

`sensorId` означає:

- Modbus slave address для датчика тиску і датчиків вологості;
- GPIO pin для лічильників води.

Поточні значення `sensorType`:

- `pressure`;
- `water_counter`;
- `soil_temperature`;
- `soil_humidity`.

`value` може бути `null`, якщо останнє читання сенсора завершилося помилкою або
значення ще недоступне.

### POST /api/valves/open-for-time

Відкриває клапан на заданий час.

Запит:

```http
POST /api/valves/open-for-time
Authorization: Bearer <apiAccessToken>
Content-Type: application/json
```

Тіло запиту:

```json
{
  "pin": 19,
  "seconds": 60
}
```

Відповідь:

```json
{
  "success": true,
  "data": {
    "pin": 19,
    "seconds": 60
  },
  "error": null
}
```

Валідація:

- `pin` має тип uint8;
- `seconds` має тип uint32;
- `seconds` має бути менше або дорівнювати
  `settings.globalSettings.maximumManualValveOpenTimeSeconds`;
- якщо valve з таким pin не існує, відповідь має статус `404 Not Found`.

### POST /api/modbus/device-address

Змінює Modbus slave address пристрою.

Запит:

```http
POST /api/modbus/device-address
Authorization: Bearer <apiAccessToken>
Content-Type: application/json
```

Тіло запиту без save register:

```json
{
  "currentAddress": 1,
  "newAddress": 2,
  "registerAddress": 256
}
```

Тіло запиту із save register:

```json
{
  "currentAddress": 1,
  "newAddress": 2,
  "registerAddress": 256,
  "saveRegisterAddress": 512,
  "saveValue": 1
}
```

Відповідь:

```json
{
  "success": true,
  "data": {
    "status": 0,
    "currentAddress": 1,
    "newAddress": 2,
    "registerAddress": 256,
    "save": true,
    "saveRegisterAddress": 512,
    "saveValue": 1
  },
  "error": null
}
```

Валідація:

- `currentAddress` і `newAddress` мають тип uint8;
- `registerAddress`, `saveRegisterAddress`, `saveValue` мають тип uint16;
- `saveRegisterAddress` і `saveValue` треба передавати разом;
- non-zero Modbus status повертає `502` як internal result і
  `error: "Failed to change device address"`.

Примітка: поточна firmware мапить невідомий status `502` у HTTP
`500 Internal Server Error` у status line відповіді, але JSON усе одно містить
Modbus `status` і `success: false`.

## Server API для мобільного додатку

Цей API реалізується backend-сервером. Він потрібен для:

- отримання історії логів;
- отримання логів через push-сповіщення;
- отримання time series і summary по метриках для графіків.

Детальний серверний контракт описаний у
`docs/server-developer-documentation.md`.

Базовий URL:

```text
https://api.example.com
```

Кожен запит має app-level authorization:

```http
Authorization: Bearer <mobile-session-token>
Content-Type: application/json
```

`mobile-session-token` - це token користувача мобільного додатку. Це не
`apiAccessToken` контролера і не `remoteLogSettings.token`.

### Реєстрація push token

Після отримання APNs/FCM token додаток реєструє його на сервері.

```http
POST /api/mobile/push-tokens
Authorization: Bearer <mobile-session-token>
Content-Type: application/json
```

```json
{
  "platform": "fcm",
  "token": "push-token-from-os",
  "appDeviceId": "mobile-installation-id"
}
```

Відповідь:

```json
{
  "success": true,
  "data": {
    "pushTokenId": "pt_123"
  },
  "error": null
}
```

### Налаштування push-підписки на логи

Додаток вмикає push-сповіщення по логах для конкретного контролера.

```http
PUT /api/mobile/devices/{deviceId}/log-push-subscription
Authorization: Bearer <mobile-session-token>
Content-Type: application/json
```

```json
{
  "enabled": true,
  "minLevel": "warning",
  "tags": ["WaterHub", "ApiServerWifi"]
}
```

Відповідь:

```json
{
  "success": true,
  "data": {
    "enabled": true,
    "minLevel": "warning",
    "tags": ["WaterHub", "ApiServerWifi"]
  },
  "error": null
}
```

Коли сервер отримує від контролера запис логу, який відповідає цій підписці, він
відправляє push.

Data payload push-сповіщення:

```json
{
  "type": "controller.log",
  "deviceId": "dev_123",
  "logId": "log_456",
  "level": "warning",
  "tag": "WaterHub",
  "message": "Pressure sensor read failed",
  "receivedAt": "2026-06-06T10:15:30Z"
}
```

Після tap на push додаток відкриває екран логів і за потреби дочитує історію
через `GET /api/mobile/devices/{deviceId}/logs`.

### Отримання історії логів

```http
GET /api/mobile/devices/{deviceId}/logs?from=2026-06-06T00:00:00Z&to=2026-06-07T00:00:00Z&minLevel=warning&limit=50&cursor=opaque
Authorization: Bearer <mobile-session-token>
```

Відповідь:

```json
{
  "success": true,
  "data": {
    "items": [
      {
        "logId": "log_456",
        "deviceId": "dev_123",
        "level": "warning",
        "uptimeMs": 123456,
        "tag": "WaterHub",
        "message": "Pressure sensor read failed",
        "receivedAt": "2026-06-06T10:15:30Z"
      }
    ],
    "nextCursor": null
  },
  "error": null
}
```

### Отримання останніх метрик

Використовувати для dashboard з останніми показниками.

```http
GET /api/mobile/devices/{deviceId}/metrics/latest
Authorization: Bearer <mobile-session-token>
```

Відповідь:

```json
{
  "success": true,
  "data": {
    "items": [
      {
        "seriesKey": "soil_humidity:1",
        "sensorId": 1,
        "sensorType": "soil_humidity",
        "name": "Soil sensor 1",
        "value": 64.2,
        "uptimeMs": 123456,
        "receivedAt": "2026-06-06T10:15:30Z"
      }
    ]
  },
  "error": null
}
```

`value` може бути `null`, якщо останній ingest для серії містив невдале читання
сенсора.

### Отримання metric series для графіків

Використовувати для графіків.

```http
GET /api/mobile/devices/{deviceId}/metrics/series?sensorType=soil_humidity&sensorId=1&from=2026-06-01T00:00:00Z&to=2026-06-07T00:00:00Z&bucket=1h&aggregation=avg
Authorization: Bearer <mobile-session-token>
```

Відповідь:

```json
{
  "success": true,
  "data": {
    "seriesKey": "soil_humidity:1",
    "sensorId": 1,
    "sensorType": "soil_humidity",
    "name": "Soil sensor 1",
    "bucket": "1h",
    "aggregation": "avg",
    "points": [
      {
        "timestamp": "2026-06-06T10:00:00Z",
        "value": 64.2,
        "count": 6
      }
    ]
  },
  "error": null
}
```

Підтримувані значення `bucket`: `raw`, `1m`, `5m`, `15m`, `1h`, `1d`.
Підтримувані значення `aggregation`: `avg`, `min`, `max`, `sum`, `last`.

Raw series може містити points із `"value": null`. Bucketed series і summary
мають будуватися тільки по числових значеннях, без трактування `null` як `0`.

Для осі X графіка треба використовувати `timestamp` із відповіді сервера. Не
використовуйте controller `uptimeMs` як absolute timestamp.

### Отримання metric summary

Використовувати для короткої статистики за період.

```http
GET /api/mobile/devices/{deviceId}/metrics/summary?sensorType=soil_humidity&sensorId=1&from=2026-06-01T00:00:00Z&to=2026-06-07T00:00:00Z
Authorization: Bearer <mobile-session-token>
```

Відповідь:

```json
{
  "success": true,
  "data": {
    "seriesKey": "soil_humidity:1",
    "sensorId": 1,
    "sensorType": "soil_humidity",
    "name": "Soil sensor 1",
    "from": "2026-06-01T00:00:00Z",
    "to": "2026-06-07T00:00:00Z",
    "min": 42.1,
    "max": 78.4,
    "avg": 63.7,
    "last": 64.2,
    "count": 144
  },
  "error": null
}
```

## Рекомендовані сценарії додатку

### Перше налаштування

1. Просканувати BLE devices за service UUID або device name.
2. Підключитися і виконати pairing із passkey `482917`.
3. Прочитати `WifiSettings` і показати поточний SSID.
4. Записати `SaveWifiSettings`.
5. Дочекатися `restartScheduled: true`.
6. Від'єднатися і зачекати device reboot.
7. Повторно підключитися через BLE.
8. Опитувати `WifiIpAddress`, доки `ipAddress` не стане відмінним від `0.0.0.0`.
9. Прочитати `ApiAccessToken`.
10. Викликати `GET https://<localHostname>/api/settings` із bearer token, якщо mDNS резолвиться; інакше використати `https://<ip>/api/settings`.
11. Якщо cloud features увімкнені, викликати server `POST /api/mobile/push-tokens`
    і `PUT /api/mobile/devices/{deviceId}/log-push-subscription`.

### Звичайний запуск додатку

1. Використати збережені `localHostname`, `ipAddress` і `apiAccessToken`.
2. Спробувати `GET /api/settings`.
3. Якщо підключення не вдалося, повторно підключитися через BLE і прочитати свіжий `WifiIpAddress`.
4. Якщо авторизація завершується статусом `401`, повторно підключитися через BLE і прочитати
   `ApiAccessToken` ще раз.

### Зміна налаштувань контролера

1. Викликати `GET /api/settings`.
2. Дати користувачу змінити settings.
3. Відправити повний об'єкт settings через `PUT /api/settings`.
4. При `restartScheduled: true` зачекати reboot.
5. Опитувати `GET /api/settings`, доки контролер знову не стане доступним.

### Перегляд логів

1. Зареєструвати push token, якщо він ще не зареєстрований.
2. Увімкнути push-підписку на логи для вибраного `deviceId`.
3. При push із `type: "controller.log"` відкрити екран логів пристрою.
4. Завантажити історію з `GET /api/mobile/devices/{deviceId}/logs`.

### Перегляд графіків метрик

1. Завантажити поточний список сенсорів із controller `GET /api/sensors/metrics` або server
   `GET /api/mobile/devices/{deviceId}/metrics/latest`.
2. Для кожного вибраного графіка викликати
   `GET /api/mobile/devices/{deviceId}/metrics/series`.
3. Використовувати bucketed data для довгих діапазонів і raw data тільки для коротких діапазонів.
4. Використовувати server timestamps на осі X.

### Ручний полив клапаном

1. Викликати `GET /api/settings` і показати valves із `settings.valveSettings`.
2. Користувач вибирає valve і duration.
3. Переконатися, що duration не більше за
   `maximumManualValveOpenTimeSeconds`.
4. Викликати `POST /api/valves/open-for-time`.
5. Опитувати `GET /api/sensors/metrics`, якщо UI потребує live feedback від сенсорів.

## Нотатки щодо реалізації клієнта

- Обробляти значення BLE characteristics як UTF-8 JSON strings.
- Після BLE write у `SaveWifiSettings` прочитати значення того самого
  characteristic або використати результат write із BLE library, якщо вона повертає
  оновлене value.
- Не виводити `apiAccessToken` у logs або crash reports.
- Зберігати `apiAccessToken` у захищеному сховищі мобільного пристрою.
- Використовувати certificate pinning для самопідписаного HTTPS endpoint.
- Під час збереження settings відправляти повний settings snapshot.
  `PUT /api/settings` замінює збережені settings, це не partial patch.
- Локально зберігати зручні для користувача дані профілю пристрою: last IP,
  token, BLE device id і display name.
- Тримати mobile server session token окремо від controller `apiAccessToken` і
  controller ingest `remoteLogSettings.token`.
