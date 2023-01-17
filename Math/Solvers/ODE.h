#pragma once


#include <math.h>
#include <array>



namespace ODE
{
	template<typename T, unsigned int N>
	struct IState
	{
		static_assert(N > 0);
		virtual void GetDerivatives(std::array<T, N>& derivatives) const = 0;
		virtual T GetNthDerivative(const std::array<T, N>& derivatives) const = 0;
		virtual void SetDerivatives(const std::array<T, N>& derivatives) = 0;
	};


	// 1ST ORDER

	template<typename T, unsigned int N>
	void ExplicitEuler(IState<T, N>& state, float stepSize)
	{
		static_assert(N > 0);

		std::array<T, N> derivatives;
		state.GetDerivatives(derivatives);
		const T nthDerivative = state.GetNthDerivative(derivatives);

		for (unsigned int i = 0; i < N - 1; ++i)
			derivatives[i] += derivatives[i + 1] * stepSize;
		derivatives[N - 1] += nthDerivative * stepSize;

		state.SetDerivatives(derivatives);
	}


	template<typename T, unsigned int N>
	void SemiImplicitEuler(IState<T, N>& state, float stepSize)
	{
		static_assert(N > 0);

		std::array<T, N> derivatives;
		state.GetDerivatives(derivatives);

		derivatives[N - 1] += state.GetNthDerivative(derivatives) * stepSize;
		for (int i = N - 2; i >= 0; --i)
			derivatives[i] += derivatives[i + 1] * stepSize;

		state.SetDerivatives(derivatives);
	}


	// 2ND ORDER

	template<typename T, unsigned int N>
	void ExplicitMidpoint(IState<T, N>& state, float stepSize)
	{
		static_assert(N > 0);
		
		std::array<T, N> derivatives;
		state.GetDerivatives(derivatives);

		std::array<T, N> k1;
		for (unsigned int i = 0; i < N - 1; ++i)
			k1[i] = derivatives[i + 1];
		k1[N - 1] = state.GetNthDerivative(derivatives);

		const float halfStepSize = 0.5f * stepSize;

		std::array<T, N> dataMid;
		for (unsigned int i = 0; i < N; ++i)
			dataMid[i] = derivatives[i] + k1[i] * halfStepSize;

		std::array<T, N> k2;
		for (unsigned int i = 0; i < N - 1; ++i)
			k2[i] = derivatives[i + 1] + k1[i + 1] * halfStepSize;
		k2[N - 1] = state.GetNthDerivative(dataMid);

		for (unsigned int i = 0; i < N; ++i)
			derivatives[i] += k2[i] * stepSize;

		state.SetDerivatives(derivatives);
	}


	template<typename T>
	void VelocityVerlet(IState<T, 2>& state, float stepSize)
	{
		std::array<T, 2> derivatives;
		state.GetDerivatives(derivatives);

		const T nthDerivative0 = state.GetNthDerivative(derivatives);
		derivatives[0] += derivatives[1] * stepSize + nthDerivative0 * stepSize * stepSize * 0.5f;

		const T nthDerivative1 = state.GetNthDerivative(derivatives);
		derivatives[1] += (nthDerivative0 + nthDerivative1) * stepSize * 0.5f;

		state.SetDerivatives(derivatives);
	}


	// 4TH ORDER

	template<typename T, unsigned int N>
	void ExplicitRK4(IState<T, N>& state, float stepSize)
	{
		static_assert(N > 0);

		std::array<T, N> derivatives;
		state.GetDerivatives(derivatives);
		
		std::array<T, N> k1;
		for (unsigned int i = 0; i < N - 1; ++i)
			k1[i] = derivatives[i + 1];
		k1[N - 1] = state.GetNthDerivative(derivatives);

		const float halfStepSize = 0.5f * stepSize;

		std::array<T, N> estimateData;
		for (unsigned int i = 0; i < N; ++i)
			estimateData[i] = derivatives[i] + k1[i] * halfStepSize;

		std::array<T, N> k2;
		for (unsigned int i = 0; i < N - 1; ++i)
			k2[i] = k1[i] + k1[i + 1] * halfStepSize;
		k2[N - 1] = state.GetNthDerivative(estimateData);

		for (unsigned int i = 0; i < N; ++i)
			estimateData[i] = derivatives[i] + k2[i] * halfStepSize;

		std::array<T, N> k3;
		for (unsigned int i = 0; i < N - 1; ++i)
			k3[i] = k1[i] + k2[i + 1] * halfStepSize;
		k3[N - 1] = state.GetNthDerivative(estimateData);

		for (unsigned int i = 0; i < N; ++i)
			estimateData[i] = derivatives[i] + k3[i] * stepSize;

		std::array<T, N> k4;
		for (unsigned int i = 0; i < N - 1; ++i)
			k4[i] = k1[i] + k3[i + 1] * stepSize;
		k4[N - 1] = state.GetNthDerivative(estimateData);

		constexpr float sixth = 1.0f / 6.0f;
		for (unsigned int i = 0; i < N; ++i)
			derivatives[i] += (k1[i] + k2[i] * 2.0f + k3[i] * 2.0f + k4[i]) * sixth * stepSize;

		state.SetDerivatives(derivatives);
	}


	template<typename T>
	void Ruth4(IState<T, 2>& state, float stepSize)
	{
		constexpr float twoToThird = 1.25992104989f;
		constexpr float ratio = 1.0f / (2.0f - twoToThird);
		constexpr float c[4] = { 0.5f * ratio, 0.5f * (1.0f - twoToThird) * ratio, 0.5f * (1.0f - twoToThird) * ratio, 0.5f * ratio };
		constexpr float d[4] = { 0.0f, ratio, -twoToThird * ratio, ratio };

		std::array<T, 2> derivatives;
		state.GetDerivatives(derivatives);

		derivatives[0] += derivatives[1] * c[0] * stepSize;
		for (int i = 1; i < 4; ++i)
		{
			const T nthDerivative = state.GetNthDerivative(derivatives);
			derivatives[1] += nthDerivative * d[i] * stepSize;
			derivatives[0] += derivatives[1] * c[i] * stepSize;
		}

		state.SetDerivatives(derivatives);
	}
};