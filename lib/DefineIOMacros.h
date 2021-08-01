// FWD(bytes): 丢弃n个字节输入
#pragma push_macro("FWD")
#define FWD(n) is.seekg(n, std::istream::cur)

// READT(x, t): 读取一个t类型的字段x，输入流必须别名为is
#pragma push_macro("READT")
#define READT(x, t)                                                            \
  {                                                                            \
    t a;                                                                       \
    is.read((char *)&a, sizeof(t));                                            \
    x = std::decay_t<decltype(x)>(a);                                          \
  }                                                                            \
  void(0) // 分号的提示

// WRITET(x, t)，输出流必须别名为os
#pragma push_macro("WRITET")
#define WRITET(x, t) { auto a = (t)x; os.write((const char*)&a, sizeof(t)); } 0
