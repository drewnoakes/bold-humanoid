%{
#include <Setting/setting.hh>
#include <Setting/setting-implementations.hh>
%}

namespace bold
{
  class SettingBase
  {
  public:
    std::string getPath() const;
    std::string getName() const;
    bool isReadOnly() const;
    bool isAdvanced() const;
    std::string getTypeName() const;
    std::string getDescription() const;

    virtual void resetToDefaultValue() = 0;
  protected:
    virtual ~SettingBase() {}

  };

  template<typename T>
  class Setting : public SettingBase
  {
  public:
    Setting(std::string path, std::string typeName, bool isReadOnly, bool isAdvanced, T value, std::string description);
    const T getValue() const;
    bool setValue(T value);
    void resetToDefaultValue();

    virtual bool isValidValue(T value) const = 0;
    virtual std::string getValidationMessage(T value) const = 0;
    virtual T getDefaultValue() const = 0;
  };

  // List template instantiations used for specific setting types below
  %template(IntSettingBase) Setting<int>;
  %template(DoubleSettingBase) Setting<double>;
  %template(BoolSettingBase) Setting<bool>;
  %template(HsvRangeSettingBase) Setting<Colour::hsvRange>;
  %template(DoubleRangeSettingBase) Setting<Range<double> >;
  %template(StringSettingBase) Setting<std::string>;

  class IntSetting : public Setting<int>
  {
  public:
    IntSetting(std::string path,
               int min, int max, int defaultValue,
               bool isReadOnly, bool isAdvanced,
               std::string description);
    ~IntSetting();

    int getMinimum() const;
    int getMaximum() const;
    bool isValidValue(int value) const;
    std::string getValidationMessage(int value) const;
    int getDefaultValue() const;
  };

  class EnumSetting : public Setting<int>
  {
  public:
    EnumSetting(std::string path,
                std::map<int,std::string> pairs, int defaultValue,
                bool isReadOnly, bool isAdvanced,
                std::string description);
    ~EnumSetting();

    bool isValidValue(int value) const;
    std::string getValidationMessage(int value) const;
    int getDefaultValue() const;
  };

  class DoubleSetting : public Setting<double>
  {
  public:
    DoubleSetting(std::string path,
                  double min, double max, double defaultValue,
                  bool isReadOnly, bool isAdvanced,
                  std::string description);
    ~DoubleSetting();

    bool isValidValue(double value) const;
    std::string getValidationMessage(double value) const;
    double getDefaultValue() const;
  };

  class BoolSetting : public Setting<bool>
  {
  public:
    BoolSetting(std::string path,
                bool defaultValue,
                bool isReadOnly, bool isAdvanced,
                std::string description);
    ~BoolSetting();

    bool isValidValue(bool value) const;
    std::string getValidationMessage(bool value) const;
    bool getDefaultValue() const;
  };

  /// Models a setting with a Colour::hsvRange value.
  class HsvRangeSetting : public Setting<Colour::hsvRange>
  {
  public:
    HsvRangeSetting(std::string path,
                    Colour::hsvRange defaultValue,
                    bool isReadOnly, bool isAdvanced,
                    std::string description);
    ~HsvRangeSetting();

    bool isValidValue(Colour::hsvRange value) const;
    std::string getValidationMessage(Colour::hsvRange value) const;
    Colour::hsvRange getDefaultValue() const;
  };

  class DoubleRangeSetting : public Setting<Range<double> >
  {
  public:
    DoubleRangeSetting(std::string path,
                       Range<double> defaultValue,
                       bool isReadOnly, bool isAdvanced,
                       std::string description);
    ~DoubleRangeSetting();

    bool isValidValue(Range<double> value) const;
    std::string getValidationMessage(Range<double> value) const;
    Range<double> getDefaultValue() const;
  };

  class StringSetting : public Setting<std::string>
  {
  public:
    StringSetting(std::string path,
                  std::string defaultValue,
                  bool isReadOnly, bool isAdvanced,
                  std::string description);
    ~StringSetting() {}

    bool isValidValue(std::string value) const;
    std::string getValidationMessage(std::string value) const;
    std::string getDefaultValue() const;
  };

}
