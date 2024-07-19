#include <intrusive_ptr.hpp>
#include <legacy.hpp>

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

struct SimpleLegacyWidget
{
    explicit SimpleLegacyWidget(std::string message)
        : data(std::move(message))
    { }

    std::string data;
};

struct DerivedLegacyWidget
    : public SimpleLegacyWidget
{
    explicit DerivedLegacyWidget(int value)
        : SimpleLegacyWidget("Derived")
        , Value(value)
    { }

    int Value;
};

struct LegacyWidgetWithSelfPointers
{
    explicit LegacyWidgetWithSelfPointers(std::ostringstream& stream, int count)
        : Stream(stream)
    {
        Stream << "1";

        for (int i = 0; i < count; ++i) {
            Stream << "2";
            IntrusivePtr<LegacyWidgetWithSelfPointers> ptr(this);
        }

        Stream << "3";
    }

    ~LegacyWidgetWithSelfPointers()
    {
        Stream << "4";
    }

    std::ostringstream& Stream;
};

struct VirtualBase1LegacyWidget
{
    virtual ~VirtualBase1LegacyWidget() = default;
};

struct VirtualBase2LegacyWidget
{
    virtual ~VirtualBase2LegacyWidget() = default;

    explicit VirtualBase2LegacyWidget(std::string message)
        : data(std::move(message))
    { }

    std::string data;
};

struct VirtualDerivedLegacyWidget
    : public VirtualBase1LegacyWidget
    , public VirtualBase2LegacyWidget
{
    explicit VirtualDerivedLegacyWidget(std::string message)
        : VirtualBase2LegacyWidget("Derived" + message)
        , derived_data(std::move(message))
    { }

    ~VirtualDerivedLegacyWidget() override = default;

    std::string derived_data;
};

struct AlignedLegacyClass
{
    alignas(64) int value;
};

namespace {

struct DontSupportMe
{ };

template <class T, class U>
concept NewConstructible = requires (U arg) {
    New<T>(arg);
};

static_assert(!NewConstructible<int, int>);
static_assert(!NewConstructible<std::string, std::string>);
static_assert(!NewConstructible<double, double>);
static_assert(!NewConstructible<DontSupportMe, DontSupportMe>);

static_assert(sizeof(IntrusivePtr<SimpleLegacyWidget>) == sizeof(void*));
static_assert(NewConstructible<SimpleLegacyWidget, std::string>);
static_assert(!NewConstructible<SimpleLegacyWidget, int>);

TEST(IntrusivePtrTestLegacy, Empty)
{
    IntrusivePtr<SimpleLegacyWidget> empty;
    EXPECT_EQ(nullptr, empty.Get());
}

TEST(IntrusivePtrTestLegacy, New)
{
    auto ptr = New<SimpleLegacyWidget>("Hello World");
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr->data, "Hello World");
}

TEST(IntrusivePtrTestLegacy, Copy)
{
    static_assert(std::copyable<SimpleLegacyWidget>);

    auto ptr = New<SimpleLegacyWidget>("CopyTest");
    EXPECT_EQ(ptr.GetRefCount(), 1);

    auto copy(ptr);

    EXPECT_EQ(copy.GetRefCount(), ptr.GetRefCount());
    EXPECT_EQ(copy.GetRefCount(), 2);
    EXPECT_EQ(copy->data, "CopyTest");
}

TEST(IntrusivePtrTestLegacy, Move)
{
    auto ptr = New<SimpleLegacyWidget>("MoveTest");
    EXPECT_EQ(ptr.GetRefCount(), 1);

    auto moved = std::move(ptr);
    EXPECT_FALSE(ptr);

    EXPECT_EQ(moved.GetRefCount(), 1);
    EXPECT_EQ(moved->data, "MoveTest");
}

TEST(IntrusivePtrTestLegacy, CopyAssignment)
{
    auto ptr = New<SimpleLegacyWidget>("AssignTest");

    auto ptr2 = New<SimpleLegacyWidget>("DiscardedMessage");

    ptr2 = ptr;

    EXPECT_EQ(ptr2.GetRefCount(), ptr.GetRefCount());
    EXPECT_EQ(ptr2.GetRefCount(), 2);
    EXPECT_EQ(ptr2->data, "AssignTest");
}

TEST(IntrusivePtrTestLegacy, MoveAssignment)
{
    auto ptr = New<SimpleLegacyWidget>("MoveAssignTest");

    auto ptr2 = New<SimpleLegacyWidget>("DiscardedMessage");

    ptr2 = std::move(ptr);

    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr2.GetRefCount(), 1);
    EXPECT_EQ(ptr2->data, "MoveAssignTest");
}

TEST(IntrusivePtrTestLegacy, SelfMove)
{
    std::vector<IntrusivePtr<SimpleLegacyWidget>> vec{New<SimpleLegacyWidget>("SelfMove")};
    std::swap(vec.front(), vec.back());
    
    auto& ptr = vec.front();
    EXPECT_EQ(ptr.GetRefCount(), 1);
    EXPECT_EQ(ptr->data, "SelfMove");
}

TEST(IntrusivePtrTestLegacy, NoPrematureDestruction)
{
    std::ostringstream stream{};

    int count = 3;

    New<LegacyWidgetWithSelfPointers>(stream, count);

    std::string expected = "1";
    for (int i = 0; i < count; ++i) {
        expected += "2";
    }

    expected += "34";

    EXPECT_EQ(stream.str(), expected);
}

TEST(IntrusivePtrTestLegacy, Upcast)
{
    IntrusivePtr<SimpleLegacyWidget> base = New<DerivedLegacyWidget>(42);
    EXPECT_EQ(base->data, "Derived");

    IntrusivePtr<DerivedLegacyWidget> drv = New<DerivedLegacyWidget>(11);
    drv->data = "Boo";
    EXPECT_EQ(drv->Value, 11);

    base = drv.StaticPointerCast<SimpleLegacyWidget>();
    EXPECT_EQ(base->data, "Boo");
}

TEST(IntrusivePtrTestLegacy, UpcastCopy)
{
    auto ptr = New<DerivedLegacyWidget>(42);
    EXPECT_EQ(ptr.GetRefCount(), 1);

    IntrusivePtr<SimpleLegacyWidget> copy(ptr);

    EXPECT_EQ(copy.GetRefCount(), ptr.GetRefCount());
    EXPECT_EQ(copy.GetRefCount(), 2);
    EXPECT_EQ(copy->data, "Derived");
}

TEST(IntrusivePtrTestLegacy, UpcastMove)
{
    auto ptr = New<DerivedLegacyWidget>(42);
    EXPECT_EQ(ptr.GetRefCount(), 1);

    IntrusivePtr<SimpleLegacyWidget> moved = std::move(ptr);
    EXPECT_FALSE(ptr);

    EXPECT_EQ(moved.GetRefCount(), 1);
    EXPECT_EQ(moved->data, "Derived");
}

TEST(IntrusivePtrTestLegacy, UpcastCopyAssignment)
{
    auto ptr = New<DerivedLegacyWidget>(42);

    auto ptr2 = New<SimpleLegacyWidget>("Discarded");

    ptr2 = ptr;

    EXPECT_EQ(ptr2.GetRefCount(), ptr.GetRefCount());
    EXPECT_EQ(ptr2.GetRefCount(), 2);
    EXPECT_EQ(ptr2->data, "Derived");
}

TEST(IntrusivePtrTestLegacy, UpcastMoveAssignment)
{
    auto ptr = New<DerivedLegacyWidget>(42);

    auto ptr2 = New<SimpleLegacyWidget>("DiscardedMessage");

    ptr2 = std::move(ptr);

    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr2.GetRefCount(), 1);
    EXPECT_EQ(ptr2->data, "Derived");
}

TEST(IntrusivePtrTestLegacy, Downcast)
{
    IntrusivePtr<SimpleLegacyWidget> base = New<DerivedLegacyWidget>(11);

    auto drv = base.StaticPointerCast<DerivedLegacyWidget>();
    EXPECT_EQ(drv->data, "Derived");
    EXPECT_EQ(drv->Value, 11);
}

TEST(IntrusivePtrTestLegacy, DynamicCastValid)
{
    IntrusivePtr<VirtualBase1LegacyWidget> base1 = New<VirtualDerivedLegacyWidget>("DynCast");
    auto drv = base1.DynamicPointerCast<VirtualDerivedLegacyWidget>();
    EXPECT_TRUE(drv);
    EXPECT_EQ(drv->derived_data, "DynCast");

    auto base2 = base1.DynamicPointerCast<VirtualBase2LegacyWidget>();
    EXPECT_TRUE(base2);
    EXPECT_EQ(base2->data, "DerivedDynCast");
}

TEST(IntrusivePtrTestLegacy, DynamicCastInvalid)
{
    IntrusivePtr<VirtualBase1LegacyWidget> base1 = New<VirtualBase1LegacyWidget>();
    auto drv = base1.DynamicPointerCast<VirtualDerivedLegacyWidget>();
    EXPECT_FALSE(drv);

    auto base2 = base1.DynamicPointerCast<VirtualBase2LegacyWidget>();
    EXPECT_FALSE(base2);
}

TEST(IntrusivePtrTestLegacy, AllocCount1)
{
    AllocCount = 0;
    DeleteCount = 0;

    New<SimpleLegacyWidget>("AllocTest");

    EXPECT_EQ(AllocCount, 1);
    EXPECT_EQ(DeleteCount, 1);
}

TEST(IntrusivePtrTestLegacy, AllocCount2)
{
    AllocCount = 0;
    DeleteCount = 0;

    {
        auto ptr = New<SimpleLegacyWidget>("AllocTest");
        EXPECT_EQ(AllocCount, 1);
        EXPECT_EQ(DeleteCount, 0);

        {
            auto another = New<SimpleLegacyWidget>("AllocTest2");

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

TEST(IntrusivePtrTestLegacy, Alignment)
{
    auto ptr = New<AlignedLegacyClass>();

    EXPECT_TRUE(reinterpret_cast<uintptr_t>(ptr.Get()) % 64 == 0);
}

TEST(IntrusivePtrTestLegacy, CompareWithOther)
{
    auto ptr1 = New<SimpleLegacyWidget>("1");
    auto ptr2 = New<SimpleLegacyWidget>("2");

    EXPECT_FALSE(ptr1 == ptr2);
    EXPECT_TRUE(ptr1 == ptr1);
}

TEST(IntrusivePtrTestLegacy, CompareWithNull)
{
    auto ptr = New<SimpleLegacyWidget>("1");

    EXPECT_FALSE(ptr == nullptr);
    EXPECT_TRUE(IntrusivePtr<SimpleLegacyWidget>() == nullptr);
}

} // namespace
