# ISSUE

## millis()

millis을 받는 타입은 `unsigned long`형태로 받아야 한다.

이렇게 받지 않고 int형으로 받았다가는 65535값을 나타내며 원치 않는 작동을 하였다.

```c++
unsigned long current_time = millis();
```

