// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

/* The NavCategory group of classes and enumerations is intended to serve
 * as a single location for storing metadata about a navigation mode.
 *
 * These .h and .cpp files:
 *     - Define the ViewMode enumeration which is used for setting the mode of the app.
 *     - Define a list of metadata associated with each ViewMode.
 *     - Define the order of groups and items in the navigation menu.
 *     - Provide a static helper function for creating the navigation menu options.
 *     - Provide static helper functions for querying information about a given ViewMode.
 */

#pragma once

#include "Utils.h"
#include "MyVirtualKey.h"

namespace CalculatorApp
{
    namespace Common
    {
        // Don't change the order of these enums
        // and definitely don't use int arithmetic
        // to change modes.
    public
        enum class ViewMode
        {
            None = -1,
            Standard = 0,
            Scientific = 1,
            Programmer = 2,
            Date = 3,
            Volume = 4,
            Length = 5,
            Weight = 6,
            Temperature = 7,
            Energy = 8,
            Area = 9,
            Speed = 10,
            Time = 11,
            Power = 12,
            Data = 13,
            Pressure = 14,
            Angle = 15,
            Currency = 16,
            Graphing = 17
        };

    public
        enum class CategoryGroupType
        {
            None = -1,
            Calculator = 0,
            Converter = 1
        };

    private
        struct NavCategoryInitializer
        {
            constexpr NavCategoryInitializer(
                ViewMode mode,
                int id,
                wchar_t const* name,
                wchar_t const* nameKey,
                wchar_t const* glyph,
                CategoryGroupType group,
                MyVirtualKey vKey,
                wchar_t const* aKey,
                bool categorySupportsNegative,
                bool enabled)
                : viewMode(mode)
                , serializationId(id)
                , friendlyName(name)
                , nameResourceKey(nameKey)
                , glyph(glyph)
                , groupType(group)
                , virtualKey(vKey)
                , accessKey(aKey)
                , supportsNegative(categorySupportsNegative)
                , isEnabled(enabled)
            {
            }

            const ViewMode viewMode;
            const int serializationId;
            const wchar_t* const friendlyName;
            const wchar_t* const nameResourceKey;
            const wchar_t* const glyph;
            const CategoryGroupType groupType;
            const MyVirtualKey virtualKey;
            const wchar_t* const accessKey;
            const bool supportsNegative;
            const bool isEnabled;
        };

    private
        struct NavCategoryGroupInitializer
        {
            constexpr NavCategoryGroupInitializer(CategoryGroupType t, wchar_t const* h, wchar_t const* n, wchar_t const* a)
                : type(t)
                , headerResourceKey(h)
                , modeResourceKey(n)
                , automationResourceKey(a)
            {
            }

            const CategoryGroupType type;
            const wchar_t* headerResourceKey;
            const wchar_t* modeResourceKey;
            const wchar_t* automationResourceKey;
        };

        [Windows::UI::Xaml::Data::Bindable] public ref class NavCategory sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged
        {
        public:
            OBSERVABLE_OBJECT();
            PROPERTY_R(Platform::String ^, Name);
            PROPERTY_R(Platform::String ^, AutomationName);
            PROPERTY_R(Platform::String ^, Glyph);
            PROPERTY_R(ViewMode, Mode);
            PROPERTY_R(Platform::String ^, AccessKey);
            PROPERTY_R(bool, SupportsNegative);
            PROPERTY_R(bool, IsEnabled);

            property Platform::String
                ^ AutomationId { Platform::String ^ get() { return m_Mode.ToString(); } }


            // For saving/restoring last mode used.
            static int Serialize(ViewMode mode);
            static ViewMode Deserialize(Platform::Object ^ obj);
            static ViewMode GetViewModeForFriendlyName(Platform::String ^ name);

            static bool IsValidViewMode(ViewMode mode);
            static bool IsViewModeEnabled(ViewMode mode);
            static bool IsCalculatorViewMode(ViewMode mode);
            static bool IsGraphingCalculatorViewMode(ViewMode mode);
            static bool IsDateCalculatorViewMode(ViewMode mode);
            static bool IsConverterViewMode(ViewMode mode);

            static Platform::String ^ GetFriendlyName(ViewMode mode);
            static Platform::String ^ GetNameResourceKey(ViewMode mode);
            static CategoryGroupType GetGroupType(ViewMode mode);

            // GetIndex is 0-based, GetPosition is 1-based
            static int GetIndex(ViewMode mode);
            static int GetFlatIndex(ViewMode mode);
            static int GetIndexInGroup(ViewMode mode, CategoryGroupType type);
            static int GetPosition(ViewMode mode);

            static ViewMode GetViewModeForVirtualKey(MyVirtualKey virtualKey);

            internal : NavCategory(
                           Platform::String ^ name,
                           Platform::String ^ automationName,
                           Platform::String ^ glyph,
                           Platform::String ^ accessKey,
                           Platform::String ^ mode,
                           ViewMode viewMode,
                           bool supportsNegative,
                           bool isEnabled)
                : m_Name(name)
                , m_AutomationName(automationName)
                , m_Glyph(glyph)
                , m_AccessKey(accessKey)
                , m_modeString(mode)
                , m_Mode(viewMode)
                , m_SupportsNegative(supportsNegative)
                , m_IsEnabled(isEnabled)
            {
            }

            static std::vector<MyVirtualKey> GetCategoryAcceleratorKeys();

        private:
            static bool IsModeInCategoryGroup(ViewMode mode, CategoryGroupType groupType);

            Platform::String ^ m_modeString;
        };

        [Windows::UI::Xaml::Data::Bindable] public ref class NavCategoryGroup sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged
        {
        public:
            OBSERVABLE_OBJECT();
            OBSERVABLE_PROPERTY_R(Platform::String ^, Name);
            OBSERVABLE_PROPERTY_R(Platform::String ^, AutomationName);
            OBSERVABLE_PROPERTY_R(CategoryGroupType, GroupType);
            OBSERVABLE_PROPERTY_R(Windows::Foundation::Collections::IObservableVector<NavCategory ^> ^, Categories);

            static Windows::Foundation::Collections::IObservableVector<NavCategoryGroup ^> ^ CreateMenuOptions();

            internal : static NavCategoryGroup ^ CreateCalculatorCategory();
            static NavCategoryGroup ^ CreateConverterCategory();

        private:
            NavCategoryGroup(const NavCategoryGroupInitializer& groupInitializer);
        };
    }
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include "NavCategory.h"
#include "AppResourceProvider.h"
#include "Common/LocalizationStringUtil.h"
#include <initializer_list>

using namespace CalculatorApp;
using namespace CalculatorApp::Common;
using namespace Concurrency;
using namespace Platform;
using namespace Platform::Collections;
using namespace std;
using namespace Windows::Foundation::Collections;
using namespace Windows::Management::Policies;
using namespace Windows::System;

namespace UCM = UnitConversionManager;

// Calculator categories always support negative and positive.
static constexpr bool SUPPORTS_ALL = true;

// Converter categories usually only support positive.
static constexpr bool SUPPORTS_NEGATIVE = true;
static constexpr bool POSITIVE_ONLY = false;

// vvv THESE CONSTANTS SHOULD NEVER CHANGE vvv
static constexpr int STANDARD_ID = 0;
static constexpr int SCIENTIFIC_ID = 1;
static constexpr int PROGRAMMER_ID = 2;
static constexpr int DATE_ID = 3;
static constexpr int VOLUME_ID = 4;
static constexpr int LENGTH_ID = 5;
static constexpr int WEIGHT_ID = 6;
static constexpr int TEMPERATURE_ID = 7;
static constexpr int ENERGY_ID = 8;
static constexpr int AREA_ID = 9;
static constexpr int SPEED_ID = 10;
static constexpr int TIME_ID = 11;
static constexpr int POWER_ID = 12;
static constexpr int DATA_ID = 13;
static constexpr int PRESSURE_ID = 14;
static constexpr int ANGLE_ID = 15;
static constexpr int CURRENCY_ID = 16;
static constexpr int GRAPHING_ID = 17;
// ^^^ THESE CONSTANTS SHOULD NEVER CHANGE ^^^

wchar_t* towchar_t(int number)
{
    auto wstr = to_wstring(number);
    return _wcsdup(wstr.c_str());
}

bool IsGraphingModeAvailable()
{
    static bool supportGraph = Windows::Foundation::Metadata::ApiInformation::IsMethodPresent("Windows.UI.Text.RichEditTextDocument", "GetMath");
    return supportGraph;
}

Box<bool> ^ _isGraphingModeEnabledCached = nullptr;
bool IsGraphingModeEnabled()
{
    if (!IsGraphingModeAvailable())
    {
        return false;
    }

    if (_isGraphingModeEnabledCached != nullptr)
    {
        return _isGraphingModeEnabledCached->Value;
    }

    User ^ firstUser;
    create_task(User::FindAllAsync(UserType::LocalUser)).then([&firstUser](IVectorView<User ^> ^ users) {
        firstUser = users->GetAt(0); }).wait();
        auto namedPolicyData = NamedPolicy::GetPolicyFromPathForUser(firstUser, L"Education", L"AllowGraphingCalculator");
        _isGraphingModeEnabledCached = namedPolicyData->GetBoolean() == true;

    return _isGraphingModeEnabledCached->Value;
}

// The order of items in this list determines the order of items in the menu.
static const list<NavCategoryInitializer> s_categoryManifest = [] {
    auto res = list<NavCategoryInitializer>{ NavCategoryInitializer{ ViewMode::Standard,
                                                                     STANDARD_ID,
                                                                     L"Standard",
                                                                     L"StandardMode",
                                                                     L"\uE8EF",
                                                                     CategoryGroupType::Calculator,
                                                                     MyVirtualKey::Number1,
                                                                     L"1",
                                                                     SUPPORTS_ALL,
                                                                     true },
                                             NavCategoryInitializer{ ViewMode::Scientific,
                                                                     SCIENTIFIC_ID,
                                                                     L"Scientific",
                                                                     L"ScientificMode",
                                                                     L"\uF196",
                                                                     CategoryGroupType::Calculator,
                                                                     MyVirtualKey::Number2,
                                                                     L"2",
                                                                     SUPPORTS_ALL,
                                                                     true } };

    int currentIndex = 3;
    bool supportGraphingCalculator = IsGraphingModeAvailable();
    if (supportGraphingCalculator)
    {
        const bool isEnabled = IsGraphingModeEnabled();
        res.push_back(NavCategoryInitializer{ ViewMode::Graphing,
                                              GRAPHING_ID,
                                              L"Graphing",
                                              L"GraphingCalculatorMode",
                                              L"\uF770",
                                              CategoryGroupType::Calculator,
                                              MyVirtualKey::Number3,
                                              L"3",
                                              SUPPORTS_ALL,
                                              isEnabled });
        ++currentIndex;
    }
    res.insert(
        res.end(),
        { NavCategoryInitializer{ ViewMode::Programmer,
                                  PROGRAMMER_ID,
                                  L"Programmer",
                                  L"ProgrammerMode",
                                  L"\uECCE",
                                  CategoryGroupType::Calculator,
                                  supportGraphingCalculator ? MyVirtualKey::Number4 : MyVirtualKey::Number3,
                                  towchar_t(currentIndex++),
                                  SUPPORTS_ALL,
                                  true },
          NavCategoryInitializer{ ViewMode::Date,
                                  DATE_ID,
                                  L"Date",
                                  L"DateCalculationMode",
                                  L"\uE787",
                                  CategoryGroupType::Calculator,
                                  supportGraphingCalculator ? MyVirtualKey::Number5 : MyVirtualKey::Number4,
                                  towchar_t(currentIndex++),
                                  SUPPORTS_ALL,
                                  true },
          NavCategoryInitializer{ ViewMode::Currency,
                                  CURRENCY_ID,
                                  L"Currency",
                                  L"CategoryName_Currency",
                                  L"\uEB0D",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true },
          NavCategoryInitializer{ ViewMode::Volume,
                                  VOLUME_ID,
                                  L"Volume",
                                  L"CategoryName_Volume",
                                  L"\uF1AA",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true },
          NavCategoryInitializer{ ViewMode::Length,
                                  LENGTH_ID,
                                  L"Length",
                                  L"CategoryName_Length",
                                  L"\uECC6",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true },
          NavCategoryInitializer{ ViewMode::Weight,
                                  WEIGHT_ID,
                                  L"Weight and Mass",
                                  L"CategoryName_Weight",
                                  L"\uF4C1",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true },
          NavCategoryInitializer{ ViewMode::Temperature,
                                  TEMPERATURE_ID,
                                  L"Temperature",
                                  L"CategoryName_Temperature",
                                  L"\uE7A3",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  SUPPORTS_NEGATIVE,
                                  true },
          NavCategoryInitializer{ ViewMode::Energy,
                                  ENERGY_ID,
                                  L"Energy",
                                  L"CategoryName_Energy",
                                  L"\uECAD",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true },
          NavCategoryInitializer{ ViewMode::Area,
                                  AREA_ID,
                                  L"Area",
                                  L"CategoryName_Area",
                                  L"\uE809",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true },
          NavCategoryInitializer{ ViewMode::Speed,
                                  SPEED_ID,
                                  L"Speed",
                                  L"CategoryName_Speed",
                                  L"\uEADA",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true },
          NavCategoryInitializer{ ViewMode::Time,
                                  TIME_ID,
                                  L"Time",
                                  L"CategoryName_Time",
                                  L"\uE917",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true },
          NavCategoryInitializer{ ViewMode::Power,
                                  POWER_ID,
                                  L"Power",
                                  L"CategoryName_Power",
                                  L"\uE945",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  SUPPORTS_NEGATIVE,
                                  true },
          NavCategoryInitializer{ ViewMode::Data,
                                  DATA_ID,
                                  L"Data",
                                  L"CategoryName_Data",
                                  L"\uF20F",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true },
          NavCategoryInitializer{ ViewMode::Pressure,
                                  PRESSURE_ID,
                                  L"Pressure",
                                  L"CategoryName_Pressure",
                                  L"\uEC4A",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true },
          NavCategoryInitializer{ ViewMode::Angle,
                                  ANGLE_ID,
                                  L"Angle",
                                  L"CategoryName_Angle",
                                  L"\uF515",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  SUPPORTS_NEGATIVE,
                                  true } });
    return res;
}();

// This function should only be used when storing the mode to app data.
int NavCategory::Serialize(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode; });

    return (iter != s_categoryManifest.end()) ? iter->serializationId : -1;
}

// This function should only be used when restoring the mode from app data.
ViewMode NavCategory::Deserialize(Platform::Object ^ obj)
{
    // If we cast directly to ViewMode we will fail
    // because we technically store an int.
    // Need to cast to int, then ViewMode.
    auto boxed = dynamic_cast<Box<int> ^>(obj);
    if (boxed != nullptr)
    {
        int serializationId = boxed->Value;
        auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [serializationId](const NavCategoryInitializer& initializer) {
            return initializer.serializationId == serializationId;
        });

        if (iter != s_categoryManifest.end())
        {
            if (iter->viewMode == ViewMode::Graphing)
            {
                // check if the user is allowed to use this feature
                if (!IsGraphingModeEnabled())
                {
                    return ViewMode::None;
                }
            }
            return iter->viewMode;
        }
    }

    return ViewMode::None;
}

bool NavCategory::IsValidViewMode(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode; });

    return iter != s_categoryManifest.end();
}

bool NavCategory::IsViewModeEnabled(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode && initializer.isEnabled; });

    return iter != s_categoryManifest.end();
}

bool NavCategory::IsCalculatorViewMode(ViewMode mode)
{
    // Historically, Calculator modes are Standard, Scientific, and Programmer.
    return !IsDateCalculatorViewMode(mode) && !IsGraphingCalculatorViewMode(mode) && IsModeInCategoryGroup(mode, CategoryGroupType::Calculator);
}

bool NavCategory::IsGraphingCalculatorViewMode(ViewMode mode)
{
    return mode == ViewMode::Graphing;
}

bool NavCategory::IsDateCalculatorViewMode(ViewMode mode)
{
    return mode == ViewMode::Date;
}

bool NavCategory::IsConverterViewMode(ViewMode mode)
{
    return IsModeInCategoryGroup(mode, CategoryGroupType::Converter);
}

bool NavCategory::IsModeInCategoryGroup(ViewMode mode, CategoryGroupType type)
{
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode, type](const NavCategoryInitializer& initializer) {
        return initializer.viewMode == mode && initializer.groupType == type;
    });

    return iter != s_categoryManifest.end();
}

String ^ NavCategory::GetFriendlyName(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode; });

    return (iter != s_categoryManifest.end()) ? StringReference(iter->friendlyName) : L"None";
}

ViewMode NavCategory::GetViewModeForFriendlyName(String ^ name)
{
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [name](const NavCategoryInitializer& initializer) {
        return wcscmp(initializer.friendlyName, name->Data()) == 0;
    });

    return (iter != s_categoryManifest.end()) ? iter->viewMode : ViewMode::None;
}

String ^ NavCategory::GetNameResourceKey(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode; });

    return (iter != s_categoryManifest.end()) ? StringReference(iter->nameResourceKey) + "Text" : nullptr;
}

CategoryGroupType NavCategory::GetGroupType(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode; });

    return (iter != s_categoryManifest.end()) ? iter->groupType : CategoryGroupType::None;
}

// GetIndex is 0-based, GetPosition is 1-based
int NavCategory::GetIndex(ViewMode mode)
{
    int position = NavCategory::GetPosition(mode);
    return max(-1, position - 1);
}

int NavCategory::GetFlatIndex(ViewMode mode)
{
    int index = -1;
    CategoryGroupType type = CategoryGroupType::None;
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode, &type, &index](const NavCategoryInitializer& initializer) {
        index++;
        if (initializer.groupType != type)
        {
            type = initializer.groupType;
            index++;
        }

        return initializer.viewMode == mode;
    });

    return (iter != s_categoryManifest.end()) ? index : -1;
}

// GetIndex is 0-based, GetPosition is 1-based
int NavCategory::GetIndexInGroup(ViewMode mode, CategoryGroupType type)
{
    int index = -1;
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode, type, &index](const NavCategoryInitializer& initializer) {
        if (initializer.groupType == type)
        {
            index++;
            return initializer.viewMode == mode;
        }

        return false;
    });

    return (iter != s_categoryManifest.end()) ? index : -1;
}

// GetIndex is 0-based, GetPosition is 1-based
int NavCategory::GetPosition(ViewMode mode)
{
    int position = 0;
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode, &position](const NavCategoryInitializer& initializer) {
        position++;
        return initializer.viewMode == mode;
    });

    return (iter != s_categoryManifest.end()) ? position : -1;
}

ViewMode NavCategory::GetViewModeForVirtualKey(MyVirtualKey virtualKey)
{
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [virtualKey](const NavCategoryInitializer& initializer) {
        return initializer.virtualKey == virtualKey;
    });

    return (iter != s_categoryManifest.end()) ? iter->viewMode : ViewMode::None;
}

vector<MyVirtualKey> NavCategory::GetCategoryAcceleratorKeys()
{
    vector<MyVirtualKey> accelerators{};
    for (auto category : s_categoryManifest)
    {
        if (category.virtualKey != MyVirtualKey::None)
        {
            accelerators.push_back(category.virtualKey);
        }
    }

    return accelerators;
}

NavCategoryGroup::NavCategoryGroup(const NavCategoryGroupInitializer& groupInitializer)
    : m_Categories(ref new Vector<NavCategory ^>())
{
    m_GroupType = groupInitializer.type;

    auto resProvider = AppResourceProvider::GetInstance();
    m_Name = resProvider->GetResourceString(StringReference(groupInitializer.headerResourceKey));
    String ^ groupMode = resProvider->GetResourceString(StringReference(groupInitializer.modeResourceKey));
    String ^ automationName = resProvider->GetResourceString(StringReference(groupInitializer.automationResourceKey));

    String ^ navCategoryHeaderAutomationNameFormat = resProvider->GetResourceString(L"NavCategoryHeader_AutomationNameFormat");
    m_AutomationName = LocalizationStringUtil::GetLocalizedString(navCategoryHeaderAutomationNameFormat, automationName);

    String ^ navCategoryItemAutomationNameFormat = resProvider->GetResourceString(L"NavCategoryItem_AutomationNameFormat");

    for (const NavCategoryInitializer& categoryInitializer : s_categoryManifest)
    {
        if (categoryInitializer.groupType == groupInitializer.type)
        {
            String ^ nameResourceKey = StringReference(categoryInitializer.nameResourceKey);
            String ^ categoryName = resProvider->GetResourceString(nameResourceKey + "Text");
            String ^ categoryAutomationName = LocalizationStringUtil::GetLocalizedString(navCategoryItemAutomationNameFormat, categoryName, m_Name);

            m_Categories->Append(ref new NavCategory(
                categoryName,
                categoryAutomationName,
                StringReference(categoryInitializer.glyph),
                categoryInitializer.accessKey != nullptr ? ref new String(categoryInitializer.accessKey)
                                                         : resProvider->GetResourceString(nameResourceKey + "AccessKey"),
                groupMode,
                categoryInitializer.viewMode,
                categoryInitializer.supportsNegative,
                categoryInitializer.isEnabled));
        }
    }
}

IObservableVector<NavCategoryGroup ^> ^ NavCategoryGroup::CreateMenuOptions()
{
    auto menuOptions = ref new Vector<NavCategoryGroup ^>();
    menuOptions->Append(CreateCalculatorCategory());
    menuOptions->Append(CreateConverterCategory());
    return menuOptions;
}

NavCategoryGroup ^ NavCategoryGroup::CreateCalculatorCategory()
{
    return ref new NavCategoryGroup(
        NavCategoryGroupInitializer{ CategoryGroupType::Calculator, L"CalculatorModeTextCaps", L"CalculatorModeText", L"CalculatorModePluralText" });
}

NavCategoryGroup ^ NavCategoryGroup::CreateConverterCategory()
{
    return ref new NavCategoryGroup(
        NavCategoryGroupInitializer{ CategoryGroupType::Converter, L"ConverterModeTextCaps", L"ConverterModeText", L"ConverterModePluralText" });
}