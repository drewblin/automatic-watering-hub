# Оновлення прошивки через Wifi

Оновлення через Wifi працює через ArduinoOTA. OTA сервіс запускається тільки
після успішного підключення контролера до Wifi. Hostname пристрою:
`automatic-watering-hub`.

Перше прошивання версії з OTA потрібно зробити через USB:
```sh
pio run -e esp32dev -t upload
```

Після цього наступні прошивки можна завантажувати через Wifi:
```sh
AUTO_WATERING_HUB_OTA_PASSWORD='<api-access-token>' \
  pio run -e esp32dev_ota -t upload --upload-port <wifi-ip-address>
```

`<wifi-ip-address>` можна прочитати через BLE характеристику
`GetWifiIpAddress`. `<api-access-token>` читається через захищену BLE
характеристику `GetApiAccessToken` і використовується як OTA пароль.

Безпека від несанкціонованого оновлення:
1. OTA захищений паролем. Без правильного `api-access-token` ArduinoOTA
   відхиляє оновлення.
2. `api-access-token` генерується випадково під час першого запуску, зберігається
   в NVS і не виводиться в HTTPS API.
3. Прочитати token можна лише через BLE після зашифрованого authenticated pairing із
   6-значним passkey.
4. Оновлення можливе тільки з тієї мережі, де доступний Wifi контролера.

Важливе обмеження: ArduinoOTA не шифрує весь трафік прошивки і не перевіряє
криптографічний підпис firmware. Тому Wifi мережа має бути довіреною, а
`api-access-token` не можна передавати стороннім. Якщо token скомпрометовано,
той, хто має доступ до мережі контролера, зможе прошити власну firmware.
