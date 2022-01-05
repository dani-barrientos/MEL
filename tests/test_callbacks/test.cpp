#include "test.h"
#include <iostream>
#include <core/CallbackSubscriptor.h>
using core::CallbackSubscriptor;
#include <mpl/Int2Type.h>
using mpl::Int2Type;

// int f1(float a)
// {
//     std::cout << "f1 "<< a << '\n';
//     return 6;
// }
// int f2(float a)
// {
//     std::cout << "f2 "<< a << '\n';
//     return 7;
// }


// typedef std::pair<Int2Type<0>,CallbackSubscriptor<float>> CS1; 
// typedef std::pair<Int2Type<1>,CallbackSubscriptor<float>> CS2; 
// class Pepe : private CS1,
//  private CS2
// {
//     public:
//     template <class T>
//      auto subscribe1(T&& f)
//      {
//          return CS1::second.subscribeCallback(std::forward<T>(f));
//      }
//     template <class T>
//      auto subscribe2(T&& f)
//      {
//          return CS2::second.subscribeCallback(std::forward<T>(f));
//      }
//       template <class T>
//      auto unsubscribe1(T&& f)
//      {
//          return CS1::second.unsubscribeCallback(std::forward<T>(f));
//      }
//     template <class T>
//      auto unsubscribe2(T&& f)
//      {
//          return CS2::second.unsubscribeCallback(std::forward<T>(f));
//      }
//      auto trigger1(float a)
//      {
//          return CS1::second.triggerCallbacks(a);
//      }
//      auto trigger2(float a)
//      {
//          return CS2::second.triggerCallbacks(a);
//      }
// };

// int test_callbacks::test()
// {
//     /*std::function<int (int,float)> f1;
//     std::function<int (int,float)> f2;
//     problema gordo: el operator == no existe en function. TEMAS:
//      - SEGUIR USANDO MIS TEMAS->TENGO QUE MEJORAR UN POCO EL SABER SI SON COMPARABLES
//      - EL PROBLEMA ES QUE EL USO DE FUNCION PARA CALLBACK SUBSCRIPTOR HACE QUE NO VALGA PARA DESUSCRIBIR. 
//      - ¿podría usar otro mecanismo para desuscripciones? POSIBILIDADES:
//         - EL ID DEVUELTO POR LA SUBSCRIPCION->ME TOCA LA GAITA ANDAR GUARDANDOLO (O BIEN QUE LA SUBSCRIPCION DEVUELVA UN OBJETO DE MAYOR NIVEL Y ES ESE El QUE SE DESUSCRIBE..). pERO ESTAMOS NE LAS MISMAS, ME OBLIGA A GUARDARLO..
//         - TAMPOCO PUEDO DISTINGUIR ENTRE USAR FUNCTION Y OTROS, PORQUE ES INCONSISTENTE (ahora devuelvo false en esas, pero es muy feo)
//         - yo quiero claramente poder hacer la desuscripción pero que falle en compilación si uso un function..->
//     f1.target() == f2.target();
//     */
// //     CallbackSubscriptor<Int2Type<0>,false,int,float> cs; 
// //     cs.subscribeCallback(f1);
// //     cs.unsubscribeCallback(f1);
// //     cs.subscribeCallback(std::function<int(float)>(f2));
// //     //cs.unsubscribeCallback(std::function<int(float)>(f2)); 
// //    // siguiente: quitar el tipo de los callbacks
// //     cs.triggerCallbacks(8);

//     Pepe pp;
//     int s1 = pp.subscribe1(f1);
//     pp.subscribe1(std::function<int(float)>(f1));
//     pp.subscribe2(std::function<int(float)>(f2));
//     //pp.unsubscribe1(f1);
//     //pp.unsubscribe1(s1);
//     pp.trigger1(5);
//     pp.trigger2(6);

  
//     //  AHORA PENSAR OTRO MECANISMO PARA DESUSCRIPCIÓN AUTOMATICA. POSIBILIDADES:
//     //   - QUE LO CALLBACKS DEVUELVAN UN PAIR<BOOL,RESULT>, DONDE EL PRIMER ELEMENTO ES LA DESUSCRIPCION -> CUIDADO SI DEVUELVE VOID->en este caso, devolver un unico valor que es la desuscripción??
//     //   o usar un tuple?? así sería un tuple de un elemento, y sería consistente todo¿¿??
//     return 0;
// }
core::ECallbackResult f1(int )
{
    return core::ECallbackResult::NO_UNSUBSCRIBE;
}
core::ECallbackResult f2(int,float )
{
    return core::ECallbackResult::NO_UNSUBSCRIBE;
}
core::ECallbackResult f3()
{
    return core::ECallbackResult::NO_UNSUBSCRIBE;
}
core::ECallbackResult f4(int,float,float,int,int,float,int )
{
    return core::ECallbackResult::NO_UNSUBSCRIBE;
}
int test_callbacks::test()
{
    
    CallbackSubscriptor<::core::NoMultithreadPolicy,int> cs;   
    CallbackSubscriptor<::core::MultithreadPolicy,int,float> cs2;
    CallbackSubscriptor<::core::NoMultithreadPolicy,void> cs3;
    CallbackSubscriptor<::core::MultithreadPolicy,int,float,float,int,int,float,int> cs4;

    cs.subscribeCallback(f1);
    cs.triggerCallbacks(1);
    cs2.subscribeCallback(f2);    
    cs2.unsubscribeCallback(f2);
    cs2.subscribeCallback( std::function< core::ECallbackResult(int,float)>(f2));
  //  cs2.unsubscribeCallback( std::function< core::ECallbackResult(int,float)>(f2));
    cs2.triggerCallbacks(2,4.5f);
    cs2.triggerCallbacks(2,4.5f);

    cs3.subscribeCallback(f3);
    cs3.triggerCallbacks();

    cs4.subscribeCallback(f4);
    return 0;
}