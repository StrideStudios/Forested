#pragma once

#include "Forested/ForestedMinimal.h"

struct FAnimMontageInstance;
struct FAlphaBlend;
//lambda, raw, static, sp, ufunction, uobject, weaklambda, threadsafesp

template <typename FDelegateType>
class FORESTED_API TDelegateWrapper {

	FDelegateType Delegate;

public:

	//Default Constructor
	TDelegateWrapper() = default;
	
	//Sets the delegate from input
	TDelegateWrapper(FDelegateType&& Delegate):
	Delegate(Delegate) {
	}

	//Sets the delegate from input
	TDelegateWrapper(const FDelegateType& Delegate):
	Delegate(Delegate) {
	}
	
	//Lambda Constructor
	template <typename FunctorType>
	TDelegateWrapper(FunctorType&& Function):
	TDelegateWrapper(FDelegateType::CreateLambda(Function)) {
	}

	//UObject Constructor
	template <class UserClass>
	TDelegateWrapper(UserClass* InObj, typename FDelegateType::template TUObjectMethodDelegate<UserClass> Function):
	TDelegateWrapper(FDelegateType::CreateUObject(InObj, Function)) {
	}

	operator FDelegateType&() {
		return Delegate;
	}
	
	operator const FDelegateType&() const {
		return Delegate;
	}

	const FDelegateType* operator*() const {
		return &Delegate;
	}

	FDelegateType* operator*() {
		return &Delegate;
	}

	const FDelegateType* operator->() const {
		return &Delegate;
	}
	
	FDelegateType* operator->() {
		return &Delegate;
	}
	
	//Move constructor
	TDelegateWrapper(TDelegateWrapper&& Other) {
		*this = MoveTemp(Other);
	}

	//Creates and initializes a new instance from an existing delegate object
	TDelegateWrapper(const TDelegateWrapper& Other) {
		*this = Other;
	}

	//Move assignment operator
	TDelegateWrapper& operator=(TDelegateWrapper&& Other) {
		Delegate = Other.Delegate;
		return *this;
	}

	//Assignment operator
	TDelegateWrapper& operator=(const TDelegateWrapper& Other) {
		Delegate = Other.Delegate;
		return *this;
	}
};