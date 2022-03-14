#include <iostream>
#include <type_traits>
#include <functional>
#include <string>
using std::string;
#include <variant>
struct ErrorInfo
{
    ErrorInfo(int aErr,std::string aMsg):error(aErr),errorMsg(std::move(aMsg)){}
    //@remarks negative error code is reserved for internal errors
    int		error;  //there was error. Error code. Very simple for now. 
    std::string errorMsg;
};
struct NotAvailable{};
template <class T,class ErrorType = ErrorInfo> class FutureValue : public std::variant<NotAvailable,T,ErrorType>
{

    static constexpr size_t ValidIdx = 1;
    typedef std::variant<NotAvailable,T,ErrorType> Base;
    public:
        FutureValue(){}
        FutureValue(const T& v):Base(v){
            std::cout<< "FutureValue copy\n";
        }
        FutureValue(T&& v):Base(std::move(v))
        {
            std::cout<< "FutureValue move\n";
        }
        FutureValue(const ErrorType& err):Base(err){}
        FutureValue(ErrorType&& err):Base(std::move(err)){}
        FutureValue(const FutureValue& v):Base(v){}
        FutureValue(FutureValue&& v):Base(std::move(v)){}
        /**
            * @brief get if has valid value
            */
        bool isValid() const{ return Base::index() == ValidIdx;}
        bool isAvailable() const{ return Base::index() != 0;}
        // wrapper for std::get.  Same rules as std::Get, so bad_variant_access is thrown if not a valid value
        T& value() {
            return std::get<T>(*this);
        }
        const T& value() const {
            return std::get<T>(*this);
        }
        const ErrorType& error() const
        {
            return std::get<ErrorType>(*this);
        }
        auto& operator=(const T& v){
            Base::operator=(v);
            std::cout<< "FutureValue asignment copy\n";
            return *this;
        }
        auto& operator=(T&& v){
            Base::operator=(std::move(v));
            std::cout<< "FutureValue asignment move\n";
            return *this;
        }
        auto& operator=(const ErrorType& v){
            Base::operator=(v);
            return *this;
        }
        auto& operator=(ErrorType&& v){
            Base::operator=(std::move(v));
            return *this;
        }
        auto& operator=(const FutureValue& v)
        {
            Base::operator=(v);
            return *this;
        }
        auto& operator=( FutureValue& v)
        {
            Base::operator=(v);
            return *this;
        }
        auto& operator=(FutureValue&& v)
        {
            Base::operator=(std::move(v));
            return *this;
        }
};
template <class T> class Future
{
public:
    typedef FutureValue<typename 
        std::conditional<
            std::is_reference<T>::value,
            //std::reference_wrapper<ResultType>,
            std::reference_wrapper<typename std::remove_reference<T>::type>,
            T>::type> ValueType;
    typedef typename std::conditional<
        std::is_reference<T>::value,
        T,
        const T&>::type ResultType;        
    Future(){}
    //@todo resolver
    Future(T value):mValue(value){
    }
    //aquí devolver según sea referencia o no
    ResultType getValue(){ return mValue.value();}
    //@todo tengo que resolver ahora el rvalue referece    
    // void setValue(ResultType value)
    // {
    //     mValue = value;
    // }
        /*como resolverlo cuando resultatype es referencia??
    void setValue(ResultType&& value)
    {
        mValue = std::move(value);
    }
    */
    //parece que sí vale!!!
    template <class U>
    void setValue(U&& value)
    {
        mValue = std::forward<U>(value);
    }

    //esto parece que funciona pero creo que no me va a hacer falta!!
    template <class U,std::enable_if_t<!std::is_reference<U>::value,bool> =false >
    void pepito(U&& val)
    {
        mValue = std::forward<T>(val);
    }
    
private:
    ValueType mValue;
};
//typedef Future<std::reference_wrapper<int>> MyFut;
typedef Future<int&> MyFut;
void func( MyFut& fut )
{
    //fut.getValue().get() = 8;
    //fut.getValue
}
// MyFut generate()
// {
//     //int a = 6;
//     int* a = new int(6);
//     MyFut fut(*a);
//     //delete a;
//     return fut;
// }


struct MyClass
{
  MyClass(){
      std::cout << "MyClass constructor\n";
  }
  MyClass(const MyClass&){
      std::cout << "MyClass copy\n";
  }
  MyClass(MyClass&& )
  {
      std::cout << "MyClass move\n";
  }
  MyClass& operator=(const MyClass& cl)
  {
      std::cout << "Assignment copy\n";
      return *this;
  }
  MyClass& operator=( MyClass&& cl)
  {
      std::cout << "Assignment move\n";
      return *this;
  }
};
int main()
{
    {
    std::cout << "test1: using MyClass\n";
    MyClass mc;
    Future<MyClass> fut;
    fut.setValue(mc); 
    std::cout << "test1 ->use getValue\n";
    fut.getValue();
    //auto mc2 = fut.getValue(); //aquí no deberia producirse hay copia,
    std::cout << "test1 -> use setValue\n";
    fut.setValue(std::move(mc)); 
    }
    {
    std::cout << "\ntest2: using int&\n";
    Future<int&> fut;
    int a=6;
    fut.setValue(a);
    std::cout << "a = "<<a << '\n';
    fut.getValue()=5;
    std::cout << "fut value = "<<fut.getValue() << '\n';
    std::cout << "a = "<<a << '\n';
    }
    {
    std::cout << "\ntest3: using int\n";
    Future<int> fut;
    int a=6;
    fut.setValue(a);
    std::cout << "a = "<<a << '\n';
    //fut.getValue()=5; error, no asignable
    std::cout << "fut value = "<<fut.getValue() << '\n';
    std::cout << "a = "<<a << '\n';
    }
    {
        std::cout << "\ntest4: using int*\n";
        Future<int*> fut;
        int *a=new int(6);
        fut.setValue(a);
        std::cout << "a = "<<*a << '\n';
        *fut.getValue()=5;
        std::cout << "fut value = "<<*fut.getValue() << '\n';
        std::cout << "a = "<<*a << '\n';
    }
    {
        std::cout << "\ntest5: using string\n";
        string str = "dani";
        Future<string> fut;
        std::cout << "assignment temporary string\n";
        fut.setValue(str);
       // fut.getValue()="pepe";
        std::cout << "fut value = "<<fut.getValue() << ". now changing value..";
        fut.setValue("pepe");
         std::cout << "fut value = "<<fut.getValue() << " Original value should be unchanged: "<<str<<'\n';
    }
    {
        std::cout << "\ntest6: using string&\n";
        string str = "dani";
        Future<string&> fut;
        fut.setValue(str);
        str = "dani new";
        //fut.setValue("dani"); not allowed
        std::cout << "Curr value = "<<fut.getValue() << " Now change referenced value..\n";
        fut.getValue()="pepe";
        std::cout << "fut value = "<<fut.getValue() << '\n';
        std::cout << "Original value = "<<fut.getValue() << " should be same as previous value\n";
    }
    //std::cout << fut.getValue()<<'\n';
  //  Future<int&> fut2;
}