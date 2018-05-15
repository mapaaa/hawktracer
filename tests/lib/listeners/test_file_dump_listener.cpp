#include <hawktracer/listeners/file_dump_listener.h>
#include <hawktracer/global_timeline.h>

#include <gtest/gtest.h>

static const char* test_file = "dump_listener_test_file_test_file";

TEST(TestFileDumpListener, InitShouldFailIfFileCanNotBeOpened)
{
    // Arrange
    HT_ErrorCode error;

    // Act
    HT_FileDumpListener* listener = ht_file_dump_listener_create("/non/existing/file", 4096u, &error);

    // Assert
    ASSERT_EQ(nullptr, listener);
    ASSERT_EQ(HT_ERR_CANT_OPEN_FILE, error);
}

TEST(TestFileDumpListener, EventShouldBeCorrectlyStoredInAFile)
{
    // Arrange
    HT_Timeline* timeline = ht_global_timeline_get();

    HT_FileDumpListener* listener = ht_file_dump_listener_create(test_file, 4096u, nullptr);
    ht_timeline_register_listener(timeline, ht_file_dump_listener_callback, listener);

    HT_DECL_EVENT(HT_Event, event);
    event.id = 32;
    event.timestamp = 9983;

    // Act
    ht_timeline_push_event(timeline, &event);
    ht_timeline_flush(timeline);
    ht_timeline_unregister_all_listeners(timeline);
    ht_file_dump_listener_destroy(listener);

    // Assert
    FILE* fp = fopen(test_file, "rb");
    ASSERT_NE(nullptr, fp);
    char buff[64];
    EXPECT_EQ(event.klass->get_size(&event), fread(buff, sizeof(char), 64, fp));
    size_t offset = 0;
#define ASSERT_FROM_BUFFER(event_value) \
    EXPECT_EQ(event_value, *(decltype(&event_value)(buff + offset))); offset += sizeof(event_value)
    ASSERT_FROM_BUFFER(event.klass->klass_id);
    ASSERT_FROM_BUFFER(event.timestamp);
    ASSERT_FROM_BUFFER(event.id);
#undef ASSERT_FROM_BUFFER

    fclose(fp);
}
TEST(TestFileDumpListener, ManyEventsShouldCauseDumpToFile)
{
    // Arrange
    HT_Timeline* timeline = ht_global_timeline_get();
    const size_t buffer_size = 4096u;

    remove(test_file);
    HT_FileDumpListener* listener = ht_file_dump_listener_create(test_file, buffer_size, nullptr);
    ht_timeline_register_listener(timeline, ht_file_dump_listener_callback, listener);

    // Act
    for (size_t i = 0; i < buffer_size; i++)
    {
        HT_DECL_EVENT(HT_Event, event);
        event.id = 1;
        event.timestamp = 1;
        ht_timeline_push_event(timeline, &event);

        ht_timeline_flush(timeline);
    }

    // Assert
    ht_timeline_unregister_all_listeners(timeline);
    ht_file_dump_listener_destroy(listener);

    FILE* fp = fopen(test_file, "rb");
    ASSERT_NE(nullptr, fp);
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fclose(fp);
    ASSERT_LT(buffer_size, file_size);
}

TEST(TestFileDumpListener, NonSerializedTimeline)
{
    // Arrange
    HT_Timeline timeline;
    ht_timeline_init(&timeline, 1024, HT_FALSE, HT_FALSE, NULL);

    HT_FileDumpListener* listener = ht_file_dump_listener_create(test_file, 4096u, nullptr);
    ht_timeline_register_listener(&timeline, ht_file_dump_listener_callback, listener);

    HT_DECL_EVENT(HT_Event, event);
    event.id = 32;
    event.timestamp = 9983;

    // Act
    ht_timeline_push_event(&timeline, &event);
    ht_timeline_flush(&timeline);
    ht_timeline_unregister_all_listeners(&timeline);
    ht_file_dump_listener_destroy(listener);

    // Assert
    FILE* fp = fopen(test_file, "rb");
    ASSERT_NE(nullptr, fp);
    char buff[64];
    EXPECT_EQ(event.klass->get_size(&event), fread(buff, sizeof(char), 64, fp));
    size_t offset = 0;
#define ASSERT_FROM_BUFFER(event_value) \
    EXPECT_EQ(event_value, *(decltype(&event_value)(buff + offset))); offset += sizeof(event_value)
    ASSERT_FROM_BUFFER(event.klass->klass_id);
    ASSERT_FROM_BUFFER(event.timestamp);
    ASSERT_FROM_BUFFER(event.id);
#undef ASSERT_FROM_BUFFER

    fclose(fp);
    ht_timeline_deinit(&timeline);
}
