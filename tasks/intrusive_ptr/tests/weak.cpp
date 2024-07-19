#include <intrusive_ptr.hpp>
#include <weak_ptr.hpp>

#include <testing/assert.hpp>

#include <sstream>

#include <gtest/gtest.h>

namespace {

struct SimpleWidget
    : public RefCountedBase<SimpleWidget>
{
    explicit SimpleWidget(std::string message)
        : data(std::move(message))
    { }

    std::string data;
};

struct WidgetWithDetectableDtor
    : public RefCountedBase<WidgetWithDetectableDtor>
{
    static inline bool DtorCalled = false;

    ~WidgetWithDetectableDtor()
    {
        DtorCalled = true;
    }
};

static_assert(sizeof(WeakPtr<SimpleWidget>) == sizeof(void*));
static_assert(std::regular<WeakPtr<SimpleWidget>>);

TEST(WeakPtrTest, Empty)
{
    WeakPtr<SimpleWidget> empty;
    EXPECT_EQ(nullptr, empty.Get());
}

TEST(WeakPtrTest, LockSuccess)
{
    auto strong = New<SimpleWidget>("Message");
    WeakPtr<SimpleWidget> weak = strong;

    auto another = weak.Lock();

    EXPECT_TRUE(another);
    EXPECT_EQ(another->data, "Message");
}

TEST(WeakPtrTest, LockFail)
{
    auto strong = New<SimpleWidget>("Message");
    WeakPtr<SimpleWidget> weak = strong;

    strong.Reset();

    EXPECT_TRUE(weak.Expired());

    auto another = weak.Lock();
    EXPECT_FALSE(another);
}

TEST(WeakPtrTest, AssingLock)
{
    auto strong = New<SimpleWidget>("Message");
    WeakPtr<SimpleWidget> weak = strong;

    strong = weak.Lock();

    EXPECT_TRUE(strong);
    EXPECT_EQ(strong->data, "Message");
}

TEST(WeakPtrTest, DtorInTime)
{
    WidgetWithDetectableDtor::DtorCalled = false;

    auto strong = New<WidgetWithDetectableDtor>();

    WeakPtr<WidgetWithDetectableDtor> weak = strong;

    strong.Reset();

    EXPECT_TRUE(WidgetWithDetectableDtor::DtorCalled);
    EXPECT_TRUE(weak.Expired());
}

TEST(WeakPtrTest, SelfMove)
{
    auto strong = New<SimpleWidget>("SelfMove");
    std::vector<WeakPtr<SimpleWidget>> vec{strong};
    std::swap(vec.front(), vec.back());
    
    auto ptr = vec.front().Lock();
    strong.Reset();

    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.GetRefCount(), 1);
    EXPECT_EQ(ptr->data, "SelfMove");
}

TEST(WeakPtrTest, CompareWithOther)
{
    auto strong1 = New<SimpleWidget>("Boo");
    auto strong2 = New<SimpleWidget>("Boooo");
    WeakPtr<SimpleWidget> ptr1 = strong1;
    WeakPtr<SimpleWidget> ptr2 = strong2;

    EXPECT_FALSE(ptr1 == ptr2);
    EXPECT_TRUE(ptr1 == ptr1);
}

TEST(WeakPtrTest, CompareWithNull)
{
    auto strong = New<SimpleWidget>("Boo");
    WeakPtr<SimpleWidget> ptr = strong;

    EXPECT_FALSE(ptr == nullptr);
    EXPECT_TRUE(WeakPtr<SimpleWidget>() == nullptr);
}

} // namespace
