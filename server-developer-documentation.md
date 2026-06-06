# Документація для розробника сервера

Цей документ описує серверний контракт для automatic-watering-hub:

- методи, якими контролер відправляє логи і метрики на сервер;
- методи, якими мобільний додаток отримує логи через push-сповіщення;
- методи, якими мобільний додаток отримує статистику метрик для графіків.

## Ролі і канали

У системі є три сторони:

- Controller - ESP32 firmware automatic-watering-hub.
- Server - backend, який приймає telemetry, зберігає її і віддає мобільному
  додатку.
- Mobile app - додаток користувача.

Controller не приймає вхідні запити від сервера. Він сам відправляє HTTPS POST
на server URL, заданий у `settings.remoteLogSettings.url`.

Mobile app працює з сервером через звичайний HTTPS API та отримує нові важливі
логи через push-сповіщення APNs/FCM.

## Controller -> Server

Базовий URL задається у налаштуваннях контролера:

```json
{
  "remoteLogSettings": {
    "url": "https://api.example.com/controller-ingest/devices/<device-id>",
    "token": "controller-ingest-token"
  }
}
```

Firmware додає до базового URL шлях:

- `/logs` для логів;
- `/metrics` для сенсорних метрик.

Якщо базовий URL закінчується на `/`, firmware не дублює символ `/`.

Кожен запит має заголовки:

```http
Content-Type: application/json
Authorization: Bearer <remoteLogSettings.token>
```

Сервер має відповідати статусом `< 400`. Firmware не читає тіло відповіді.
На боці firmware timeout підключення і запиту становить 1500 ms.

### POST /logs

Приймає один запис логу.

Запит:

```http
POST /controller-ingest/devices/<device-id>/logs
Authorization: Bearer <controller-ingest-token>
Content-Type: application/json
```

Тіло запиту:

```json
{
  "level": "warning",
  "uptimeMs": 123456,
  "tag": "WaterHub",
  "message": "Pressure sensor read failed"
}
```

Поля:

| Поле | Тип | Обов'язкове | Опис |
| --- | --- | --- | --- |
| `level` | string | так | `error`, `warning`, `info`, `debug`, `verbose` |
| `uptimeMs` | uint32 | так | Час від старту контролера в мілісекундах |
| `tag` | string | так | Джерело логу, до 31 символу у firmware |
| `message` | string | так | Повідомлення, до 223 символів у firmware |

Рекомендована відповідь:

```http
204 No Content
```

Поведінка сервера:

- знайти controller/device за bearer token або path parameter;
- зберегти `receivedAt` серверним UTC timestamp;
- зберегти оригінальний `uptimeMs`, бо це не absolute time;
- створити `logId`;
- якщо log відповідає push policy користувача, відправити push-сповіщення.

### POST /metrics

Приймає одну точку сенсорної метрики.

Запит:

```http
POST /controller-ingest/devices/<device-id>/metrics
Authorization: Bearer <controller-ingest-token>
Content-Type: application/json
```

Тіло запиту:

```json
{
  "sensorId": 1,
  "sensorType": "soil_humidity",
  "name": "Soil sensor 1",
  "value": 64.2,
  "uptimeMs": 123456
}
```

Поля:

| Поле | Тип | Обов'язкове | Опис |
| --- | --- | --- | --- |
| `sensorId` | uint8 | так | Modbus slave address або GPIO pin |
| `sensorType` | string | так | Тип сенсора |
| `name` | string | так | Назва сенсора для відображення |
| `value` | number або null | так | Значення виміру; `null`, якщо останнє читання сенсора завершилося помилкою або значення недоступне |
| `uptimeMs` | uint32 | так | Час від старту контролера в мілісекундах |

Поточні значення `sensorType`:

- `pressure`;
- `water_counter`;
- `soil_temperature`;
- `soil_humidity`.

Рекомендована відповідь:

```http
204 No Content
```

Поведінка сервера:

- зберегти raw metric point із `receivedAt`;
- зберегти `sensorId`, `sensorType`, `name`, `value`, `uptimeMs`;
- для графіків і summary пропускати `null` values у числових агрегаціях;
- для графіків використовувати `receivedAt` як absolute timestamp;
- не покладатися на `uptimeMs` для timeline між reboot;
- для `water_counter` зберігати cumulative value і за потреби рахувати delta/rate
  на сервері.

## Mobile App -> Server

Нижче описаний backend API, який треба реалізувати для мобільного додатку.
Це API не є частиною firmware.

Базовий URL:

```text
https://api.example.com
```

Кожен запит мобільного додатку повинен мати app-level authorization:

```http
Authorization: Bearer <mobile-session-token>
Content-Type: application/json
```

Формат відповідей:

```json
{
  "success": true,
  "data": {},
  "error": null
}
```

### POST /api/mobile/push-tokens

Реєструє push token мобільного пристрою.

Запит:

```http
POST /api/mobile/push-tokens
Authorization: Bearer <mobile-session-token>
Content-Type: application/json
```

Тіло запиту:

```json
{
  "platform": "fcm",
  "token": "push-token-from-os",
  "appDeviceId": "mobile-installation-id"
}
```

Поля:

- `platform`: `fcm` або `apns`;
- `token`: push token, виданий OS/provider;
- `appDeviceId`: стабільний id інсталяції додатку.

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

### DELETE /api/mobile/push-tokens/{pushTokenId}

Видаляє push token, наприклад при logout або вимкненні сповіщень.

Відповідь:

```json
{
  "success": true,
  "data": {},
  "error": null
}
```

### PUT /api/mobile/devices/{deviceId}/log-push-subscription

Налаштовує, які логи контролера треба надсилати користувачу через push.

Запит:

```http
PUT /api/mobile/devices/{deviceId}/log-push-subscription
Authorization: Bearer <mobile-session-token>
Content-Type: application/json
```

Тіло запиту:

```json
{
  "enabled": true,
  "minLevel": "warning",
  "tags": ["WaterHub", "ApiServerWifi"]
}
```

Поля:

- `enabled`: увімкнути або вимкнути push для логів цього контролера;
- `minLevel`: мінімальний рівень для push, рекомендовано `warning`;
- `tags`: optional whitelist. Якщо масив порожній або поле відсутнє, сервер
  застосовує тільки `minLevel`.

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

Рекомендований порядок рівнів:

```text
error > warning > info > debug > verbose
```

### Push payload для логів контролера

Сервер відправляє push через APNs/FCM, коли отриманий log контролера відповідає
підписці користувача.

Data payload:

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

Notification title/body можна сформувати так:

- title: `Automatic Watering Hub: warning`;
- body: `<tag>: <message>`.

Push payload має бути коротким. Повний запис логу додаток може дочитати через
`GET /api/mobile/devices/{deviceId}/logs`.

### GET /api/mobile/devices/{deviceId}/logs

Повертає історію логів для екрану журналу.

Запит:

```http
GET /api/mobile/devices/{deviceId}/logs?from=2026-06-06T00:00:00Z&to=2026-06-07T00:00:00Z&minLevel=warning&limit=50&cursor=opaque
Authorization: Bearer <mobile-session-token>
```

Параметри query:

| Parameter | Обов'язковий | Опис |
| --- | --- | --- |
| `from` | ні | Нижня межа UTC ISO timestamp |
| `to` | ні | Верхня межа UTC ISO timestamp |
| `minLevel` | ні | `error`, `warning`, `info`, `debug`, `verbose` |
| `tag` | ні | Фільтр за одним tag |
| `limit` | ні | Розмір сторінки, за замовчуванням `50`, максимум `200` |
| `cursor` | ні | Cursor з попередньої відповіді |

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

### GET /api/mobile/devices/{deviceId}/metrics/latest

Повертає останні значення по кожній серії сенсора.

Запит:

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

`seriesKey` має мати формат `${sensorType}:${sensorId}`.
`value` може бути `null`, якщо останній ingest для серії містив невдале читання
сенсора.

### GET /api/mobile/devices/{deviceId}/metrics/series

Повертає time series для малювання графіків.

Запит:

```http
GET /api/mobile/devices/{deviceId}/metrics/series?sensorType=soil_humidity&sensorId=1&from=2026-06-01T00:00:00Z&to=2026-06-07T00:00:00Z&bucket=1h&aggregation=avg
Authorization: Bearer <mobile-session-token>
```

Параметри query:

| Parameter | Обов'язковий | Опис |
| --- | --- | --- |
| `sensorType` | так | Тип сенсора |
| `sensorId` | так | Id сенсора |
| `from` | так | Нижня межа UTC ISO timestamp |
| `to` | так | Верхня межа UTC ISO timestamp |
| `bucket` | ні | `raw`, `1m`, `5m`, `15m`, `1h`, `1d`; за замовчуванням `raw` |
| `aggregation` | ні | `avg`, `min`, `max`, `sum`, `last`; ігнорується для `raw` |

Відповідь для bucketed data:

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

Відповідь для raw data:

```json
{
  "success": true,
  "data": {
    "seriesKey": "soil_humidity:1",
    "sensorId": 1,
    "sensorType": "soil_humidity",
    "name": "Soil sensor 1",
    "bucket": "raw",
    "aggregation": null,
    "points": [
      {
        "timestamp": "2026-06-06T10:15:30Z",
        "value": 64.2,
        "uptimeMs": 123456
      }
    ]
  },
  "error": null
}
```

Raw series може містити points із `"value": null`. Для bucketed data сервер має
рахувати `avg`, `min`, `max`, `sum`, `last` і `count` тільки по числових
значеннях.

### GET /api/mobile/devices/{deviceId}/metrics/summary

Повертає коротку статистику за період.

Запит:

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

Для `water_counter` summary може додатково містити deltas:

```json
{
  "totalDelta": 120.5,
  "first": 1000.0,
  "last": 1120.5
}
```

## Нотатки щодо зберігання

- `receivedAt` має генерувати сервер під час ingest.
- Зберігати raw points; якщо dataset виросте, bucketed aggregates можна будувати асинхронно.
- Тримати логи і метрики розділеними за `deviceId`.
- Використовувати controller ingest token тільки для Controller -> Server.
- Використовувати окрему mobile auth для Mobile App -> Server.
- Не передавати controller ingest token у мобільний додаток.
- Push-сповіщення не повинні містити секрети.
- Для осі X на графіку використовувати `receivedAt`, а не `uptimeMs`.
- Використовувати `uptimeMs` для визначення reboot контролера: якщо значення
  різко зменшилося для того самого пристрою, ймовірно, стався reboot.
