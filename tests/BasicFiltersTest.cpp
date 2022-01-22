#include <array>
#include "catch2/catch2.hpp"

#include <sst/filters.h>

TEST_CASE("Basic Filters")
{
    using sst::filters::FilterType, sst::filters::FilterSubType;

    constexpr bool printRMSs = false;

    constexpr auto sampleRate = 48000.0f;
    constexpr int blockSize = 2048;

    constexpr float A440 = 69.0f;
    constexpr int numTestFreqs = 5;
    constexpr std::array<float, numTestFreqs> testFreqs { 80.0f, 200.0f, 440.0f, 1000.0f, 10000.0f };

    auto runSine = [](auto &filterState, auto &filterUnitPtr, float testFreq, int numSamples) {
        // reset filter state
        std::fill (filterState.R, &filterState.R[sst::filters::n_filter_registers], _mm_setzero_ps());

        std::vector<float> y(numSamples, 0.0f);
        for (int i = 0; i < numSamples; ++i)
        {
            auto x = (float)std::sin(2.0 * M_PI * (double)i * testFreq / sampleRate);

            auto yVec = filterUnitPtr(&filterState, _mm_set_ps1(x));

            float yArr alignas(16)[4];
            _mm_store_ps (yArr, yVec);
            y[i] = yArr[0];
        }

        auto squareSum = std::inner_product(y.begin(), y.end(), y.begin(), 0.0f);
        auto rms = std::sqrt(squareSum / (float) numSamples);
        return 20.0f * std::log10 (rms);
    };

    struct TestConfig
    {
        FilterType type;
        FilterSubType subType;
        std::array<float, numTestFreqs> expRmsDBs;
    };

    auto runTest = [=] (const TestConfig& testConfig)
    {
        auto filterState = sst::filters::QuadFilterUnitState{};
        auto filterUnitPtr = sst::filters::GetQFPtrFilterUnit(testConfig.type, testConfig.subType);

        sst::filters::FilterCoefficientMaker coefMaker;
        coefMaker.setSampleRateAndBlockSize(sampleRate, blockSize);
        coefMaker.MakeCoeffs(A440, 0.5f, testConfig.type, testConfig.subType, nullptr, false);
        coefMaker.castCoefficients(filterState.C, filterState.dC);

        std::array<float, numTestFreqs> actualRMSs {};
        for (int i = 0; i < numTestFreqs; ++i)
        {
            auto rmsDB = runSine(filterState, filterUnitPtr, testFreqs[i], blockSize);
//            REQUIRE(rmsDB == Approx(testConfig.expRmsDBs[i]).margin(1.0e-4f));
            actualRMSs[i] = rmsDB;
        }

        if constexpr (printRMSs)
        {
            std::cout << "{ ";
            for (int i = 0; i < numTestFreqs; ++i)
                std::cout << actualRMSs[i] << "f, ";

            std::cout << "}" << std::endl;
        }
    };

    SECTION("LP_12") {
        runTest ({ FilterType::fut_lp12, FilterSubType::st_SVF, { -8.02604f, -6.72912f, -3.8718f, -20.6177f, -53.7828f } });
        runTest ({ FilterType::fut_lp12, FilterSubType::st_Rough, { -8.02604f, -6.72912f, -3.8718f, -20.6177f, -53.7828f } });
        runTest ({ FilterType::fut_lp12, FilterSubType::st_Smooth, { -8.02604f, -6.72912f, -3.8718f, -20.6177f, -53.7828f } });
    }

    SECTION("LP_24") {
        runTest ({ FilterType::fut_lp24, FilterSubType::st_SVF, { -7.79654f, -5.2026f, -1.93057f, -27.0258f, -50.9426f } });
        runTest ({ FilterType::fut_lp24, FilterSubType::st_Rough, { -7.79654f, -5.2026f, -1.93057f, -27.0258f, -50.9426f } });
        runTest ({ FilterType::fut_lp24, FilterSubType::st_Smooth, { -7.79654f, -5.2026f, -1.93057f, -27.0258f, -50.9426f } });
    }

    SECTION("HP_12") {
        runTest ({ FilterType::fut_hp12, FilterSubType::st_SVF, { -35.8899f, -20.1651f, -3.91549f, -6.78058f, -8.34447f } });
        runTest ({ FilterType::fut_hp12, FilterSubType::st_Rough, { -35.8899f, -20.1651f, -3.91549f, -6.78058f, -8.34447f } });
        runTest ({ FilterType::fut_hp12, FilterSubType::st_Smooth, { -35.8899f, -20.1651f, -3.91549f, -6.78058f, -8.34447f } });
    }

    SECTION("HP_24") {
        runTest ({ FilterType::fut_hp24, FilterSubType::st_SVF, { -38.0661f, -27.0505f, -1.89886f, -5.27993f, -8.33136f } });
        runTest ({ FilterType::fut_hp24, FilterSubType::st_Rough, { -38.0661f, -27.0505f, -1.89886f, -5.27993f, -8.33136f } });
        runTest ({ FilterType::fut_hp24, FilterSubType::st_Smooth, { -38.0661f, -27.0505f, -1.89886f, -5.27993f, -8.33136f } });
    }

    SECTION("BP_12") {
        runTest ({ FilterType::fut_bp12, FilterSubType::st_SVF, { -22.9694f, -13.5409f, -3.81424f, -13.7711f, -34.7354f } });
        runTest ({ FilterType::fut_bp12, FilterSubType::st_Rough, { -22.9694f, -13.5409f, -3.81424f, -13.7711f, -34.7354f } });
        runTest ({ FilterType::fut_bp12, FilterSubType::st_Smooth, { -22.9694f, -13.5409f, -3.81424f, -13.7711f, -34.7354f } });
    }

    SECTION("BP_24") {
        runTest ({ FilterType::fut_bp24, FilterSubType::st_SVF, { -33.6982f, -18.0861f, -1.37863f, -18.7074f, -50.5816f } });
        runTest ({ FilterType::fut_bp24, FilterSubType::st_Rough, { -33.6982f, -18.0861f, -1.37863f, -18.7074f, -50.5816f } });
        runTest ({ FilterType::fut_bp24, FilterSubType::st_Smooth, { -33.6982f, -18.0861f, -1.37863f, -18.7074f, -50.5816f } });
    }

    SECTION("NOTCH_12") {
        runTest ({ FilterType::fut_notch12, FilterSubType::st_Notch, { -3.76908f, -5.30151f, -24.7274f, -4.76339f, -3.02366f } });
        runTest ({ FilterType::fut_notch12, FilterSubType::st_NotchMild, { -3.76908f, -5.30151f, -24.7274f, -4.76339f, -3.02366f } });
    }

    SECTION("NOTCH_24") {
        runTest ({ FilterType::fut_notch24, FilterSubType::st_Notch, { -4.0896f, -4.12088f, -4.06745f, -3.19854f, -3.01547f } });
        runTest ({ FilterType::fut_notch24, FilterSubType::st_NotchMild, { -4.0896f, -4.12088f, -4.06745f, -3.19854f, -3.01547f } });
    }

    SECTION("APF") {
        runTest ({ FilterType::fut_apf, FilterSubType{}, { -4.0896f, -4.12088f, -4.06745f, -3.19854f -3.01547f } });
    }

    SECTION("LP_MOOG") {
        // @TODO: according to FilterConfigurations, lpmoog has 4 sub-types... but what are they??
        runTest ({ FilterType::fut_lpmoog, FilterSubType{}, { -8.26648f, -4.43419f, 4.17682f, -6.93047f, -25.634f } });
    }

    SECTION("SNH") {
        runTest ({ FilterType::fut_SNH, FilterSubType{}, { -6.75229f, -6.85575f, -6.83264f, -6.83216f, -5.31153f } });
    }
}