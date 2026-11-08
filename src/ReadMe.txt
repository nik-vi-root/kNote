mkdir -p run && g++ main.cpp -o run/notes_app $(pkg-config --cflags --libs Qt5Widgets sqlite3) -fPIC -std=c++17 && (cd run && ./notes_app)
Команда компиляции проекта