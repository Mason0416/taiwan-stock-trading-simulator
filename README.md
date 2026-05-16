# Stock Trading Simulator

A stock trading simulator built with C++ and Raylib.

This project simulates a simplified stock market trading system with a modern TradingView-inspired interface. Users can buy and sell stocks, track portfolio performance, and view simulated market price movements in real time.

---

# Features

- Real-time simulated stock price updates
- Bullish / Neutral / Bearish market trends
- Buy and sell system
- Whole share and fractional share trading
- Portfolio tracking
- Trade history page
- Dynamic price chart
- Account overview system
- TradingView-style UI
- Built with Raylib

---

# Project Structure

```text
main.cpp
TradingSystem.h
TradingSystem.cpp

GUI.h
GUI.cpp

Button.h
Button.cpp

StockDropdown.h
StockDropdown.cpp

StockData.h
Account.h
```

---

# Classes

## TradingSystem

Main controller of the application.

Handles:

- Game loop
- Market simulation
- Time system
- Trading logic
- Rendering
- UI updates

## StockData

Stores stock-related information.

Includes:

- stock price
- open price
- high / low
- volume
- volatility
- price history
- market trend

## Account

Stores player account data.

Includes:

- cash
- owned shares
- total assets
- trade history

## Button

Reusable UI button component.

Supports:

- hover effects
- active state
- click detection

## StockDropdown

Simple stock selection dropdown UI.

---

# Trading Logic

The stock price uses:

- random noise
- trend drift
- volatility simulation

Three market trends:

- Bullish
- Neutral
- Bearish

Each trading day randomly selects a market trend.

---

# Controls

| Action | Control |
|---|---|
| Buy Stock | Buy Button |
| Sell Stock | Sell Button |
| Next Day | Next Day Button |
| Change Order Size | + / - Buttons |
| Account Page | Avatar Button |
| Trade History | History Button |

---

# Technologies Used

- C++
- Raylib
- Object-Oriented Programming (OOP)

---

# Future Improvements

- Multiple stocks
- Candlestick charts
- News event system
- Technical indicators
- Saving/loading accounts
- Better market simulation
- Multiplayer trading competition

---

# Author

Yu-Hao Wei  
National Chengchi University  
Department of Computer Science
