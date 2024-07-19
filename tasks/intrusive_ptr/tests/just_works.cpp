#include <intrusive_ptr.hpp>

#include <testing/assert.hpp>

#include <sstream>

#include <gtest/gtest.h>

static int AllocCount = 0;
static int DeleteCount = 0;

void* operator new(std::size_t sz)
{
    ++AllocCount;

    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success
 
    if (void *ptr = std::malloc(sz))
        return ptr;
 
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}

void operator delete(void* ptr) noexcept
{
    ++DeleteCount;
    std::free(ptr);
}

namespace {

struct SimpleWidget
    : public RefCountedBase<SimpleWidget>
{
    explicit SimpleWidget(std::string message)
        : data(std::move(message))
    { }

    std::string data;
};

struct DerivedWidget
    : public SimpleWidget
    , public RefCountedBase<DerivedWidget>
{
    explicit DerivedWidget(int value)
        : SimpleWidget("Derived")
        , Value(value)
    { }

    int Value;
};

struct WidgetWithSelfPointers
    : public RefCountedBase<WidgetWithSelfPointers>
{
    explicit WidgetWithSelfPointers(std::ostringstream& stream, int count)
        : Stream(stream)
    {
        Stream << "1";

        for (int i = 0; i < count; ++i) {
            Stream << "2";
            IntrusivePtr<WidgetWithSelfPointers> ptr(this);
        }

        Stream << "3";
    }

    ~WidgetWithSelfPointers()
    {
        Stream << "4";
    }

    std::ostringstream& Stream;
};

struct VirtualBase1Widget
    : public RefCountedBase<VirtualBase1Widget>
{
    virtual ~VirtualBase1Widget() = default;
};

struct VirtualBase2Widget
    : public RefCountedBase<VirtualBase2Widget>
{
    virtual ~VirtualBase2Widget() = default;

    explicit VirtualBase2Widget(std::string message)
        : data(std::move(message))
    { }

    std::string data;
};

struct VirtualDerivedWidget
    : public VirtualBase1Widget
    , public VirtualBase2Widget
    , public RefCountedBase<VirtualDerivedWidget>
{
    explicit VirtualDerivedWidget(std::string message)
        : VirtualBase2Widget("Derived" + message)
        , derived_data(std::move(message))
    { }

    ~VirtualDerivedWidget() override = default;

    std::string derived_data;
};

template <class T, class U>
concept NewConstructible = requires (U arg) {
    New<T>(arg);
};

static_assert(!NewConstructible<int, int>);
static_assert(!NewConstructible<std::string, std::string>);
static_assert(!NewConstructible<double, double>);

static_assert(sizeof(IntrusivePtr<SimpleWidget>) == sizeof(void*));
static_assert(NewConstructible<SimpleWidget, std::string>);
static_assert(!NewConstructible<SimpleWidget, int>);

static_assert(std::regular<IntrusivePtr<SimpleWidget>>);

TEST(IntrusivePtrTest, Empty)
{
    IntrusivePtr<SimpleWidget> empty;
    EXPECT_EQ(nullptr, empty.Get());
}

TEST(IntrusivePtrTest, New)
{
    auto ptr = New<SimpleWidget>("Hello World");
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr->data, "Hello World");
}

TEST(IntrusivePtrTest, Copy)
{
    static_assert(!std::copyable<SimpleWidget>);

    auto ptr = New<SimpleWidget>("CopyTest");
    EXPECT_EQ(ptr.GetRefCount(), 1);

    auto copy(ptr);

    EXPECT_EQ(copy.GetRefCount(), ptr.GetRefCount());
    EXPECT_EQ(copy.GetRefCount(), 2);
    EXPECT_EQ(copy->data, "CopyTest");
}

TEST(IntrusivePtrTest, Move)
{
    auto ptr = New<SimpleWidget>("MoveTest");
    EXPECT_EQ(ptr.GetRefCount(), 1);

    auto moved = std::move(ptr);
    EXPECT_FALSE(ptr);

    EXPECT_EQ(moved.GetRefCount(), 1);
    EXPECT_EQ(moved->data, "MoveTest");
}

TEST(IntrusivePtrTest, CopyAssignment)
{
    auto ptr = New<SimpleWidget>("AssignTest");

    auto ptr2 = New<SimpleWidget>("DiscardedMessage");

    ptr2 = ptr;

    EXPECT_EQ(ptr2.GetRefCount(), ptr.GetRefCount());
    EXPECT_EQ(ptr2.GetRefCount(), 2);
    EXPECT_EQ(ptr2->data, "AssignTest");
}

TEST(IntrusivePtrTest, MoveAssignment)
{
    auto ptr = New<SimpleWidget>("MoveAssignTest");

    auto ptr2 = New<SimpleWidget>("DiscardedMessage");

    ptr2 = std::move(ptr);

    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr2.GetRefCount(), 1);
    EXPECT_EQ(ptr2->data, "MoveAssignTest");
}

TEST(IntrusivePtrTest, SelfMove)
{
    std::vector<IntrusivePtr<SimpleWidget>> vec{New<SimpleWidget>("SelfMove")};
    std::swap(vec.front(), vec.back());
    
    auto& ptr = vec.front();
    EXPECT_EQ(ptr.GetRefCount(), 1);
    EXPECT_EQ(ptr->data, "SelfMove");
}

TEST(IntrusivePtrTest, NoPrematureDestruction)
{
    std::ostringstream stream{};

    int count = 3;

    New<WidgetWithSelfPointers>(stream, count);

    std::string expected = "1";
    for (int i = 0; i < count; ++i) {
        expected += "2";
    }

    expected += "34";

    EXPECT_EQ(stream.str(), expected);
}

TEST(IntrusivePtrTest, Upcast)
{
    IntrusivePtr<SimpleWidget> base = New<DerivedWidget>(42);
    EXPECT_EQ(base->data, "Derived");

    IntrusivePtr<DerivedWidget> drv = New<DerivedWidget>(11);
    drv->data = "Boo";
    EXPECT_EQ(drv->Value, 11);

    base = drv.StaticPointerCast<SimpleWidget>();
    EXPECT_EQ(base->data, "Boo");
}

TEST(IntrusivePtrTest, UpcastCopy)
{
    auto ptr = New<DerivedWidget>(42);
    EXPECT_EQ(ptr.GetRefCount(), 1);

    IntrusivePtr<SimpleWidget> copy(ptr);

    EXPECT_EQ(copy.GetRefCount(), ptr.GetRefCount());
    EXPECT_EQ(copy.GetRefCount(), 2);
    EXPECT_EQ(copy->data, "Derived");
}

TEST(IntrusivePtrTest, UpcastMove)
{
    auto ptr = New<DerivedWidget>(42);
    EXPECT_EQ(ptr.GetRefCount(), 1);

    IntrusivePtr<SimpleWidget> moved = std::move(ptr);
    EXPECT_FALSE(ptr);

    EXPECT_EQ(moved.GetRefCount(), 1);
    EXPECT_EQ(moved->data, "Derived");
}

TEST(IntrusivePtrTest, UpcastCopyAssignment)
{
    auto ptr = New<DerivedWidget>(42);

    auto ptr2 = New<SimpleWidget>("Discarded");

    ptr2 = ptr;

    EXPECT_EQ(ptr2.GetRefCount(), ptr.GetRefCount());
    EXPECT_EQ(ptr2.GetRefCount(), 2);
    EXPECT_EQ(ptr2->data, "Derived");
}

TEST(IntrusivePtrTest, UpcastMoveAssignment)
{
    auto ptr = New<DerivedWidget>(42);

    auto ptr2 = New<SimpleWidget>("DiscardedMessage");

    ptr2 = std::move(ptr);

    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr2.GetRefCount(), 1);
    EXPECT_EQ(ptr2->data, "Derived");
}

TEST(IntrusivePtrTest, Downcast)
{
    IntrusivePtr<SimpleWidget> base = New<DerivedWidget>(11);

    auto drv = base.StaticPointerCast<DerivedWidget>();
    EXPECT_EQ(drv->data, "Derived");
    EXPECT_EQ(drv->Value, 11);
}

TEST(IntrusivePtrTest, DynamicCastValid)
{
    IntrusivePtr<VirtualBase1Widget> base1 = New<VirtualDerivedWidget>("DynCast");
    auto drv = base1.DynamicPointerCast<VirtualDerivedWidget>();
    EXPECT_TRUE(drv);
    EXPECT_EQ(drv->derived_data, "DynCast");

    auto base2 = base1.DynamicPointerCast<VirtualBase2Widget>();
    EXPECT_TRUE(base2);
    EXPECT_EQ(base2->data, "DerivedDynCast");
}

TEST(IntrusivePtrTest, DynamicCastInvalid)
{
    IntrusivePtr<VirtualBase1Widget> base1 = New<VirtualBase1Widget>();
    auto drv = base1.DynamicPointerCast<VirtualDerivedWidget>();
    EXPECT_FALSE(drv);

    auto base2 = base1.DynamicPointerCast<VirtualBase2Widget>();
    EXPECT_FALSE(base2);
}

TEST(IntrusivePtrTest, AllocCount1)
{
    AllocCount = 0;
    DeleteCount = 0;

    New<SimpleWidget>("AllocTest");

    EXPECT_EQ(AllocCount, 1);
    EXPECT_EQ(DeleteCount, 1);
}

TEST(IntrusivePtrTest, AllocCount2)
{
    AllocCount = 0;
    DeleteCount = 0;

    {
        auto ptr = New<SimpleWidget>("AllocTest");
        EXPECT_EQ(AllocCount, 1);
        EXPECT_EQ(DeleteCount, 0);

        {
            auto another = New<SimpleWidget>("AllocTest2");

            EXPECT_EQ(AllocCount, 2);
            EXPECT_EQ(DeleteCount, 0);

            another = ptr;

            EXPECT_EQ(AllocCount, 2);
            EXPECT_EQ(DeleteCount, 1);
        }

        EXPECT_EQ(AllocCount, 2);
        EXPECT_EQ(DeleteCount, 1);
    }

    EXPECT_EQ(AllocCount, 2);
    EXPECT_EQ(DeleteCount, 2);
}

TEST(IntrusivePtrTest, Alignment)
{
    struct AlignedClass
        : public RefCountedBase<AlignedClass>
    {
        alignas(64) int value;
    };

    auto ptr = New<AlignedClass>();

    EXPECT_TRUE(reinterpret_cast<uintptr_t>(ptr.Get()) % 64 == 0);
}

TEST(IntrusivePtrTest, CompareWithOther)
{
    auto ptr1 = New<SimpleWidget>("1");
    auto ptr2 = New<SimpleWidget>("2");

    EXPECT_FALSE(ptr1 == ptr2);
    EXPECT_TRUE(ptr1 == ptr1);
}

TEST(IntrusivePtrTest, CompareWithNull)
{
    auto ptr = New<SimpleWidget>("1");

    EXPECT_FALSE(ptr == nullptr);
    EXPECT_TRUE(IntrusivePtr<SimpleWidget>() == nullptr);
}

} // namespace
