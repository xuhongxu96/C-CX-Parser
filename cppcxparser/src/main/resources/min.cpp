namespace CalculatorApp
{
    [Windows::UI::Xaml::Data::Bindable] public ref class NavCategory sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged
    {
    public:
        PROPERTY_R(Platform::String ^, Name);

        property Platform::String ^ AutomationId { Platform::String ^ get() { return m_Mode.ToString(); } } private : Platform::String ^ m_modeString;
    };
} // namespace CalculatorApp
