//
// Created by baitianyu on 2/7/25.
//

#ifndef RPCFRAME_ABSTRACT_PROTOCOL_H
#define RPCFRAME_ABSTRACT_PROTOCOL_H

#include <memory>

namespace mrpc {

    enum ProtocolType {
        TinyPB_Protocol = 1,
        HTTP_Protocol = 2
    };

    // 为什么要用 enable_shared_from_this？
    // 1. 需要在类对象的内部中获得一个指向当前对象的 shared_ptr 对象。
    // 2. 如果在一个程序中，对象内存的生命周期全部由智能指针来管理。在这种情况下，要在一个类的成员函数中，对外部返回 this 指针就成了一个很棘手的问题。
    // 例如：若一个类 T 继承自 std::enable_shared_from_this<T> ，则 T 类中有继承自父类的成员函数： shared_from_this 。
    // 当 T 类的对象 t 被一个为名为 pt 的 std::shared_ptr 类对象管理时，
    // 调用 T::shared_from_this 成员函数，将会返回一个新的 std::shared_ptr 对象，它与 pt 共享 t 的所有权。
    // 当一个类被共享智能指针 share_ptr 管理，且在类的成员函数里需要把当前类对象作为参数传给其他函数时，这时就需要传递一个指向自身的 share_ptr。
    // struct AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol> {

    struct AbstractProtocol {
    public:
        using abstract_pro_sptr_t_ = std::shared_ptr<AbstractProtocol>;

        AbstractProtocol() = default;

        virtual ~AbstractProtocol() = default;

    public:
        std::string m_msg_id; // 请求号，唯一标识一个请求或者响应
    };
}

#endif //RPCFRAME_ABSTRACT_PROTOCOL_H
