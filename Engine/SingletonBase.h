#pragma once

template <typename T>
class SingletonBase
{
public:
	static T& GetInstance() { static T instance; return instance; }

protected:
	SingletonBase() = default;
	~SingletonBase() = default;

private:
	SingletonBase(const SingletonBase&) = delete;
	SingletonBase& operator=(const SingletonBase&) = delete;
	SingletonBase(SingletonBase&&) = delete;
	SingletonBase& operator=(SingletonBase&&) = delete;
};