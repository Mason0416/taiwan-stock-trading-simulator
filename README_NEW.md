# Virtual Stock Trading System

A modern, professional-grade financial trading simulator built with C++ and raylib featuring real-time stock price charts, portfolio tracking, and an intuitive dark-themed UI.

## 架構概覽

### 核心模組

```
Virtual Stock Trading System
│
├── TradingSystem (Main Orchestrator)
│   ├── Stock (Price Management)
│   ├── Player (Trader Profile)
│   │   └── Account (Portfolio & Trading)
│   ├── GUI (User Interface)
│   │   ├── UITheme (Color & Style)
│   │   ├── FontManager (Typography)
│   │   ├── Button (UI Component)
│   │   ├── Dropdown (Selection Widget)
│   │   └── ChartRenderer (Visualization)
│   └── EventSystem (Market News)
```

## 檔案結構

```
assignment-5-Mason0416/
├── CMakeLists.txt
├── include/
│   ├── Account.h         # 帳戶與交易記錄
│   ├── Button.h          # 按鈕與下拉菜單組件
│   ├── ChartRenderer.h   # 圖表繪製
│   ├── EventSystem.h     # 市場事件系統
│   ├── FontManager.h     # 字型管理
│   ├── GUI.h             # 主UI類別
│   ├── Stock.h           # 股票價格管理
│   ├── TradingSystem.h   # 遊戲主控制器
│   └── UITheme.h         # 配色和主題
├── src/
│   ├── Account.cpp
│   ├── Button.cpp
│   ├── ChartRenderer.cpp
│   ├── EventSystem.cpp
│   ├── FontManager.cpp
│   ├── GUI.cpp
│   ├── Stock.cpp
│   ├── TradingSystem.cpp
│   ├── UITheme.cpp
│   └── main.cpp
└── resources/
    └── fonts/           # TTF字型位置（可選）
```

## 類別職責

### 1. **UITheme** - 視覺設計系統
- 定義所有配色（Dark Mode）
- 動畫參數配置
- 陰影和漸層設定

### 2. **FontManager** - 字型系統
- 加載TTF字型
- Fallback至預設字型
- 文字寬度計算

### 3. **Stock** - 股票價格管理
- 實時價格更新
- 趨勢分析（Bullish/Neutral/Bearish）
- 波動率與動量計算
- 價格歷史追蹤（最多200筆）

### 4. **Account** - 帳戶管理
- 現金與持股追蹤
- 買賣執行
- 交易紀錄保存
- P&L計算
- 資產曲線記錄

### 5. **Player** - 玩家資料
- 玩家名稱
- 帳戶包含

### 6. **EventSystem** - 市場事件
- 隨機新聞生成
- 事件顏色分類（正面/中立/負面）
- 價格影響因素

### 7. **ChartRenderer** - 圖表渲染
- 平滑價格線繪製
- 網格線與軸標籤
- 漲跌顏色指示（綠/紅）
- 脈衝動畫效果
- 資產曲線圖

### 8. **Button** - UI按鈕元件
- 圓角設計
- Hover動畫
- 陰影效果
- 點擊檢測

### 9. **Dropdown** - 下拉菜單
- 展開/收起動畫
- 選項懸停高亮
- 整股/零股切換

### 10. **GUI** - 主UI管理
- 頁面管理（主交易頁/帳戶頁/交易紀錄頁）
- 版面布局
- 元件整合
- 轉換動畫

### 11. **TradingSystem** - 主程式控制
- 初始化與主迴圈
- 輸入處理
- 時間管理
- 買賣邏輯協調
- 頁面切換

## 功能特性

### 主交易頁面

**左側面板：股票資訊**
- 股票名稱與代號
- 產業分類
- 當前趨勢
- 實時價格與漲跌百分比
- 今日高/低價
- 波動率與動量指標

**中央區域：價格圖表**
- 平滑曲線繪製
- 自動縮放
- 網格背景
- 軸線標籤
- 漲跌顏色指示
- 脈衝動畫高亮

**右側面板：交易控制**
- 實時時間顯示
- 現金餘額
- 持股數量
- 總資產值
- 未實現P&L
- Buy / Sell / Next Day 按鈕
- 整股/零股下拉菜單

**下方橫幅：市場事件**
- 隨機新聞標題
- 彩色編碼（綠/灰/紅）
- 自動消失計時

### 帳戶頁面
- 初始資金顯示
- 現金餘額
- 持股統計
- 當前股價
- 總資產值
- 未實現P&L與百分比
- History按鈕導航

### 交易記錄頁面
- 時間戳記
- 買/賣操作類型
- 成交數量與價格
- 交易後現金結餘

## 設計模式

### SOLID原則應用

**S - Single Responsibility**
- 每個類別只有一個職責
- Stock不處理GUI，GUI不處理價格邏輯

**O - Open/Closed**
- 擴展新功能無需修改現有代碼

**L - Liskov Substitution**
- 一致的介面設計

**I - Interface Segregation**
- 最小化類別依賴

**D - Dependency Inversion**
- 高層模組不依賴低層細節

### 架構特性

- **分離關注點**：UI與業務邏輯分離
- **模組化設計**：易於新增功能
- **常量正確性**：const correctness應用
- **封裝**：私有成員與公開介面
- **資源管理**：RAII模式

## 編譯方法

### 需要

- CMake >= 3.14
- C++17編譯器
- raylib 5.0+（自動下載）

### 編譯步驟

```bash
cd assignment-5-Mason0416
mkdir build
cd build
cmake ..
cmake --build .
./OOP_Raylib_Lab
```

或使用Visual Studio Code的CMake Tools擴展：
- 開啟Command Palette (Cmd+Shift+P)
- 選擇 "CMake: Build"

## 操作說明

### 主交易頁面
- **Buy按鈕**：購買股票（整股或零股）
- **Sell按鈕**：出售股票
- **Next Day按鈕**：推進到下一個交易日
- **Dropdown菜單**：切換整股/零股模式
- **右上P按鈕**：進入帳戶頁面

### 帳戶頁面
- **History按鈕**：查看交易記錄
- **Back按鈕**：返回主頁面

### 交易紀錄頁面
- **Back按鈕**：返回帳戶頁面

## 技術亮點

✨ **現代UI/UX**
- Dark Mode主題
- 科技感設計
- 平滑動畫轉換
- 專業的金融交易介面

📊 **高級功能**
- 實時價格更新
- 波動率與動量分析
- 市場趨勢模擬
- 隨機新聞事件

🎨 **視覺設計**
- 圓角按鈕
- Hover效果
- 陰影與深度
- 自適應圖表縮放

🏗️ **軟體工程**
- 清晰的OOP架構
- 完整的模組化設計
- 易於維護和擴展
- 完整的類別文檔

## 擴展可能性

- 多個股票支持
- 真實API連接
- 更複雜的事件系統
- 資產配置優化
- 機器學習預測
- 網路多人交易
- 完整的技術分析工具
- 組合管理功能

## 作者

Created as an OOP Raylib Assignment

## 許可證

Educational Use
