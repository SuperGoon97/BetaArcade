#include "BPFL/CamiLibEngineBPFL.h"
#include "ShaderPipelineCache.h"
#include <random>

int32 UCamiLibEngineBPLibrary::GetNumPrecompilingPSOsRemaining() {
    return FShaderPipelineCache::NumPrecompilesRemaining();
}

FString UCamiLibEngineBPLibrary::InsertCommas(int64 AbsAmount) {
    FString RawString = FString::FromInt(AbsAmount);
    FString FormattedString = "";
    int32 Len = RawString.Len();

    // Iterate backwards and insert commas every 3 digits
    for (int32 i = 0; i < Len; ++i) {
        if (i > 0 && i % 3 == 0) {
            FormattedString = "," + FormattedString;
        }
        FormattedString = FString::Chr(RawString[Len - 1 - i]) + FormattedString;
    }

    return FormattedString;
}

FText UCamiLibEngineBPLibrary::FormatLargeCurrency(int64 Amount, int32 Decimals = -1, bool bUseCSV = false) {
    bool bIsNegative = Amount < 0;
    int64 AbsAmount = bIsNegative ? -Amount : Amount;

    // Numbers below 100k (or if CSV format is explicitly requested) displays with commas.
    if (AbsAmount < 100000LL || bUseCSV) {
        FString FormattedString = InsertCommas(AbsAmount);
        if (bIsNegative) {
            FormattedString = "-" + FormattedString;
        }
        return FText::FromString(FormattedString);
    }

    const TArray<FString> Suffixes = {
        TEXT("K"),
        TEXT("M"),
        TEXT("B"),
        TEXT("T"),
        TEXT("Q") // Quadrillion as our maximum value.
    };

    int32 DigitCount = FMath::FloorToInt(std::log10(static_cast<double>(AbsAmount))) + 1;

    // Map the digit count to the correct suffix index.
    // 10,000 (5 Digits) -> (5 - 1) / 3 - 1 = 0 -> Suffixes[0] ("K")
    // 100,000 (6 digits) -> (6 - 1) / 3 - 1 = 0 -> Suffixes[0] ("K")
    // 1,000,000 (7 digits) -> (7 - 1) / 3 - 1 = 1 -> Suffixes[1] ("M")
    // 1,000,000,000 (10 digits) -> (10 - 1) / 3 - 1 = 2 -> Suffixes[2] ("B")
    // 1,000,000,000,000 (13 digits) -> (13 - 1) / 3 - 1 = 3 -> Suffixes[3] ("T")
    int32 TargetIndex = FMath::Clamp((DigitCount - 1) / 3 - 1, 0, Suffixes.Num() - 1);

    // Calculate the exact divisor based on the target index (Index 0 is 1,000, Index 1 is 1,000,000)
    // (TargetIndex + 1) * 3 (If our target index is 0) will return 3. (0 + 1) * 3 = 3
    // (TargetIndex + 1) * 3 (If our target index is 1) will return 6. (1 + 1) * 3 = 6
    // If our target index is 0 but our value is 100,000. It will get 1,000 as our divisor.
    // Pow(10, 3) = 1,000
    // Pow(10, 6) = 1,000,000
    double Divisor = FMath::Pow(10.0, (TargetIndex + 1) * 3);
    // Then we use this to calcuilate the divisor. In this case, 100,000 / 1000 = 100.
    double Value = static_cast<double>(AbsAmount) / Divisor;

    // Determine decimal formatting dynamically.
    int32 DynamicDecimals;
    if (Decimals > -1) {
        DynamicDecimals = Decimals;
    } else {
        // Log10 works like so:
        // Log10(10) = 1 (10^1 = 10)
        // Log10(100) = 2 (10^2 = 100)
        // Log10(1000) = 3 (10^3 = 1000)
        // So it calculates the actual decimal number. Using floor to int returns greater values as whole numbers.
        int32 LeadingDigits = FMath::FloorToInt(std::log10(Value)) + 1;
        // We then subtract the number of digits from our smaller calculated Value. (100)
        // This should, then, equal 0 because our value that we calculated above is 100. (3 - 3 = 0)
        // If our value was 10, it would be (3 - 2 = 1) and we would have 10.0k
        // And if it was 1, it would be (3 - 1 = 2) and we would have 1.00k
        // Although all of our values are displayed as CSV under 100k regardless. (1,000, 10,000, 99,999)
        DynamicDecimals = FMath::Clamp(3 - LeadingDigits, 0, 2);
    }

    // Format string and append suffix using our dynamic decimals.
    FString FormattedString = FString::Printf(TEXT("%.*f"), DynamicDecimals, Value) + Suffixes[TargetIndex];

    // Prepend negative if necessary.
    if (bIsNegative) {
        FormattedString = "-" + FormattedString;
    }

    return FText::FromString(FormattedString);
}

int64 UCamiLibEngineBPLibrary::GetRandomLargeInteger(int64 Min, int64 Max) {
    if (Min > Max) {
        int64 Temp = Min;
        Min = Max;
        Max = Temp;
    }

    static std::random_device RD;
    static std::mt19937_64 Generator(RD());
    std::uniform_int_distribution<int64> Distribution(Min, Max);

    return Distribution(Generator);
}

double UCamiLibEngineBPLibrary::DivideInt64(int64 a, int64 b) {
    return (b > 0) ? (static_cast<double>(a) / b) : 0.0;
}
