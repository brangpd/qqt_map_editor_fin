// fwd(bytes): 丢弃n个字节输入
#pragma push_macro("fwd")
#define fwd(n) is.seekg(n, std::istream::cur)

// readt(x, t): 读取一个t类型的字段x
#pragma push_macro("readt")
#define readt(x, t)                                                            \
  {                                                                            \
    t a;                                                                       \
    is.read((char *)&a, sizeof(t));                                            \
    (x) = std::decay_t<decltype(x)>(a);                                        \
  }                                                                            \
  void(0) // 分号的提示
