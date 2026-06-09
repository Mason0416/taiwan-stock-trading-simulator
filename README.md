# Taiwan Stock Trading Simulator

這是一個使用 C++、Raylib 和 CMake 製作的台股交易模擬器。專案透過物件導向設計，把股票資料、帳戶、交易系統、新聞事件和圖形介面拆成不同類別，模擬即時股價變化、買賣交易、投資組合與交易紀錄。

詳細的專案說明可以參考 [PROJECT_DOCUMENTATION.md](PROJECT_DOCUMENTATION.md)。

## Project Features

- 即時模擬股票價格變化
- 多支股票與不同產業分類
- 買進、賣出與持股管理
- 現金、資產與交易紀錄追蹤
- 市場新聞事件影響股價走勢
- 台股漲跌停與紅漲綠跌顯示邏輯
- Raylib 圖形化操作介面
- 使用 CMake 管理建置流程

## Project Structure

```text
.
├── CMakeLists.txt
├── PROJECT_DOCUMENTATION.md
├── README.md
├── include/
│   ├── Account.h
│   ├── Button.h
│   ├── GUI.h
│   ├── NewsEvent.h
│   ├── StockData.h
│   ├── StockDropdown.h
│   └── TradingSystem.h
├── resources/
│   └── fonts/
└── src/
    ├── Account.cpp
    ├── Button.cpp
    ├── GUI.cpp
    ├── StockDropdown.cpp
    ├── TradingSystem.cpp
    └── main.cpp
```

## Build And Run

在專案根目錄執行：

```bash
cmake -S . -B build
cmake --build build
./build/OOP_Raylib_Lab
```

如果重新編譯後遇到奇怪錯誤，可以先刪除 `build` 資料夾再重新建置：

```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

## Notes

- `src/` 放主要 C++ 程式碼。
- `include/` 放標頭檔。
- `resources/` 放字型或其他資源。
- `PROJECT_DOCUMENTATION.md` 放完整專案說明。
- `build/` 是 CMake 編譯產生的資料夾，通常不需要手動修改，也不建議上傳。

## Author

Yu-Hao Wei  
National Chengchi University  
Department of Computer Science
