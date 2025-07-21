


## 📖 Правила Мьюта и размьюта

---


### Мьютим тест, если за последние 3 дня:
- **2 и более падений**
- **ИЛИ** 1 падение и запусков (pass + fail) не более 10

### Размьючиваем тест, если за последние 4 дня:
- **Запусков (pass + fail + mute) > 4**
- **И нет ни одного падения (fail + mute = 0)**

### Удаляем из mute, если за последние 7 дней:
- **Не было ни одного запуска** (pass + fail + mute + skip = 0)

---

### Примечания
- Для всех правил учитываются только последние N дней (N=3 для mute, N=2 для unmute, N=7 для delete), включая текущий день.
- Под "запуском" понимается любое выполнение теста с результатом pass или fail.
- Под "падением" понимается выполнение теста с результатом fail.
- Агрегация статистики ведётся по ключу (test_name, suite_folder, full_name, build_type, branch).

---

**Пример:**
- Если за 3 дня тест запускался 5 раз, из них 2 раза упал — тест будет замьючен.
- Если за 2 дня тест запускался 5 раз и все прошли успешно — тест будет размьючен.
- Если за 7 дней тест не запускался ни разу — он будет удалён из mute. 

## 📝 Ручное управление mute/unmute

### Как замьютить тест вручную

- Откройте [muted_ya.txt](https://github.com/ydb-platform/ydb/blob/main/.github/config/muted_ya.txt) и добавьте строку теста.
- Создайте PR, скопируйте название и описание из issue.
- Получите подтверждение от владельца теста.
- После мержа свяжите PR и issue, уведомите команду.

**Можно также:**
- Использовать контекстное меню в отчёте PR (см. скриншот).
- Использовать [Test history dashboard](https://datalens.yandex/4un3zdm0zcnyr?tab=A4) для поиска и мьюта теста.

### Как размьютить тест вручную

- Откройте [muted_ya.txt](https://github.com/ydb-platform/ydb/blob/main/.github/config/muted_ya.txt) и удалите строку теста.
- Создайте PR с названием "UnMute {testname}".
- Получите подтверждение от владельца теста.
- После мержа переведите issue в статус Unmuted, свяжите PR и issue. 

## 📊 Дашборд для анализа замьюченных и флейки тестов

Для анализа состояния тестов, поиска кандидатов на mute/unmute и отслеживания стабильности используйте интерактивный дашборд:

- [YDB Test Analytics Dashboard](https://datalens.yandex/4un3zdm0zcnyr)

**Возможности дашборда:**
- Просмотр всех замьюченных тестов по owner, full_name, статусу
- Быстрый поиск по имени теста или команде (owner)
- Фильтрация по статусу (flaky, muted, stable и др.)
- История запусков и падений по дням
- Таблицы кандидатов на mute/unmute (см. соответствующие вкладки)
- Быстрый переход к созданию mute-issue через ссылку в таблице

**Примеры использования:**
- Найти все замьюченные тесты своей команды: выберите owner в фильтре
- Найти флейки-кандидаты на mute: вкладка Flaky, фильтр по fail_count/run_count
- Найти стабильные mute для размьюта: вкладка Stable, фильтр по success_rate

## 📋 Файлы которые генерирует create_new_muted_ya.py

### 🔇 [to_mute.txt](mute_update/to_mute.txt)
**Содержимое:** Кандидаты на мьют по новым правилам  
**Правила:** За 3 дня ≥2 падения **ИЛИ** (≥1 падение и запусков ≤10)  
**Использование:** Основной файл для принятия решений о мьюте

### 🔊 [to_unmute.txt](mute_update/to_unmute.txt)
**Содержимое:** Кандидаты на размьют по новым правилам  
**Правила:** За 4 дня >4 запусков (pass+fail+mute) и нет падений (fail+mute=0)  
**Использование:** Основной файл для принятия решений о размьюте

### 🗑️ [to_remove_from_mute.txt](mute_update/to_remove_from_mute.txt)
**Содержимое:** Тесты, которые нужно удалить из mute  
**Правила:** За 7 дней нет запусков  
**Использование:** Основной файл для удаления из mute

## 📊 Дополнительные файлы для анализа

### 🔍 [muted_ya-deleted.txt](mute_update/muted_ya-deleted.txt)
**Содержимое:** Тесты из muted_ya минус удалённые тесты  
**Формула:** `muted_ya` - `deleted`  
**Использование:** Анализ активных тестов в mute

### 🔍 [muted_ya-stable.txt](mute_update/muted_ya-stable.txt)
**Содержимое:** Тесты из muted_ya минус стабильные тесты  
**Формула:** `muted_ya` - `stable`  
**Использование:** Анализ нестабильных тестов в mute

### 🔍 [muted_ya-stable-deleted.txt](mute_update/muted_ya-stable-deleted.txt)
**Содержимое:** Тесты из muted_ya минус стабильные и удалённые  
**Формула:** `muted_ya` - `stable` - `deleted`  
**Использование:** Анализ активных нестабильных тестов

### 🔍 [muted_ya-stable-deleted+flaky.txt](mute_update/muted_ya-stable-deleted+flaky.txt)
**Содержимое:** Тесты из muted_ya минус стабильные и удалённые, плюс flaky  
**Формула:** `muted_ya` - `stable` - `deleted` + `flaky`  
**Использование:** Создание GitHub issues

## 📋 Debug-файлы (с подробностями)

### 🔍 [muted_ya-deleted_debug.txt](mute_update/muted_ya-deleted_debug.txt)
**Содержимое:** Подробности по тестам muted_ya - deleted  
**Дополнительно:** owner, success_rate, state, days_in_state

### 🔍 [muted_ya-stable_debug.txt](mute_update/muted_ya-stable_debug.txt)
**Содержимое:** Подробности по тестам muted_ya - stable  
**Дополнительно:** owner, success_rate, state, days_in_state

### 🔍 [muted_ya-stable-deleted_debug.txt](mute_update/muted_ya-stable-deleted_debug.txt)
**Содержимое:** Подробности по тестам muted_ya - stable - deleted  
**Дополнительно:** owner, success_rate, state, days_in_state

### 🔍 [muted_ya-stable-deleted+flaky_debug.txt](mute_update/muted_ya-stable-deleted+flaky_debug.txt)
**Содержимое:** Подробности по тестам muted_ya - stable - deleted + flaky  
**Дополнительно:** owner, success_rate, state, days_in_state, pass_count, fail_count



---

## 🔄 Жизненный цикл файлов

1. **Анализ данных** → Создание основных файлов действий
2. **Применение правил** → Формирование трёх основных файлов
3. **Дополнительный анализ** → Создание файлов для анализа различных комбинаций
4. **Создание issues** → Использование `new_muted_ya.txt`

**Все файлы создаются в директории `mute_update/` при запуске скрипта. Итоговый mute-файл для workflow — это `new_muted_ya.txt`.** # Таблица выходных файлов mute-логики

Эта таблица показывает все файлы, создаваемые скриптом mute-логики, с описанием их содержимого и назначения.

## 📋 Основные файлы

| Файл | Описание | Правила | Использование |
|------|----------|---------|---------------|
| `to_mute.txt` | Кандидаты на мьют | За 3 дня ≥2 падения **ИЛИ** (≥1 падение и запусков ≤10) | Основной файл для принятия решений о мьюте |
| `to_unmute.txt` | Кандидаты на размьют | За 4 дня >4 запусков (pass+fail+mute) и нет падений (fail+mute=0) | Основной файл для принятия решений о размьюте |
| `to_remove_from_mute.txt` | Тесты для удаления из mute | За 7 дней нет запусков | Основной файл для удаления из mute |

## 📊 Дополнительные файлы для анализа

| Файл | Описание | Формула | Использование |
|------|----------|---------|---------------|
| `muted_ya.txt` | Все замьюченные тесты на текущий момент | агрегировано за 3 дня | База для анализа mute |
| `muted_ya+to_mute.txt` | muted_ya + to_mute | | Анализ потенциальных mute |
| `muted_ya-to_unmute.txt` | muted_ya - to_unmute | | Анализ потенциальных размьютов |
| `muted_ya-to_delete.txt` | muted_ya - to_delete | | Анализ потенциальных удалений |
| `muted_ya-to-delete-to-unmute.txt` | muted_ya - to_delete - to_unmute | | Анализ активных mute |
| `muted_ya-to-delete-to-unmute+to_mute.txt` | (muted_ya - to_delete - to_unmute) + to_mute | | Для итогового mute-файла |
| `new_muted_ya.txt` | Итоговый mute-файл для workflow (дублирует muted_ya-to-delete-to-unmute+to_mute.txt) | копия предыдущего | Используется для автоматического обновления .github/config/muted_ya.txt |

## 📋 Debug-файлы (с подробностями)

| Файл | Описание | Дополнительная информация |
|------|----------|---------------------------|
| `muted_ya-deleted_debug.txt` | Подробности по тестам muted_ya - deleted | owner, success_rate, state, days_in_state |
| `muted_ya-stable_debug.txt` | Подробности по тестам muted_ya - stable | owner, success_rate, state, days_in_state |
| `muted_ya-stable-deleted_debug.txt` | Подробности по тестам muted_ya - stable - deleted | owner, success_rate, state, days_in_state |
| `muted_ya-stable-deleted+flaky_debug.txt` | Подробности по тестам muted_ya - stable - deleted + flaky | owner, success_rate, state, days_in_state, pass_count, fail_count |

---