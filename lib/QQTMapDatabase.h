#pragma once

#include <vector>

enum class EQQTCity;
struct QQTMapElement;
struct QQFDIMG;

class QQTMapDatabase
{
public:
  class Provider
  {
  public:
    virtual ~Provider() = default;
    virtual const std::vector<int>& getAllMapElementIds() = 0;
    virtual const QQTMapElement* getMapElementById(int id) = 0;
    virtual const QQFDIMG* getQqfdimgOfMapElementById(int id) = 0;
  };

  static Provider* getProvider() { return _provider; }
  static void setProvider(Provider *provider) { _provider = provider; }

private:
  static Provider *_provider;
};
