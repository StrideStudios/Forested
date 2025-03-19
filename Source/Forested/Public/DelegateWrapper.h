#pragma once

struct FAnimMontageInstance;
struct FAlphaBlend;
//lambda, raw, static, sp, ufunction, uobject, weaklambda, threadsafesp

template <typename FDelegateType>
class FORESTED_API TDelegateWrapper : public FDelegateType {

public:

	//Null Constructor
	TDelegateWrapper() = default;
	
	//Lambda Constructor
	template <typename FunctorType>
	TDelegateWrapper(FunctorType&& Function):
	FDelegateType(FDelegateType::CreateLambda(Function)) {
	}

	//UObject Constructor
	template <class UserClass>
	TDelegateWrapper(UserClass* InObj, typename FDelegateType::template TUObjectMethodDelegate<UserClass> Function):
	FDelegateType(FDelegateType::CreateUObject(InObj, Function)) {
	}
	
};