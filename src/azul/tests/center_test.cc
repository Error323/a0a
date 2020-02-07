#include <gtest/gtest.h>

#include "azul/center.cc"

TEST(CenterTest, Reset) {
  Center c;
  c.Reset();
}

/*
class CenterTest : public ::testing::Test {

  protected:
    World &world_;

    WorldTest() : world_(World::Instance()) {
    }

    virtual void SetUp() override {
        auto &config = Configuration::Instance();
        config.Set("camera.screen_size", MakeSize<CameraSpace>(4608_px, 3288_px));
        config.Set("camera.field_of_view", MakeVector<CameraSpace>(77.8_deg, 60.1_deg));
        config.Set("camera.position", MakePoint<WorldSpace>(-10.2_m, -1.0_m, 4.95_m));
        config.Set("camera.rotation", MakeVector<WorldSpace>(170_deg, 10_deg, 5_deg));
    };

    virtual void TearDown() override{};
};

TEST_F(WorldTest, SingleInstance) {
    World &a = World::Instance();
    World &b = World::Instance();
    EXPECT_EQ(&a, &b);
}

TEST_F(WorldTest, CameraCoordinateToCameraPixelAtOrigin) {
    CamCoord cc(0_m, 0_m, 10_cm);
    CameraPixel cp = world_.ToCameraPixel(cc);
    EXPECT_NEAR((float)cp.x, 4608 / 2.0, 1e-6f);
    EXPECT_NEAR((float)cp.y, 3288 / 2.0, 1e-6f);
}

TEST_F(WorldTest, CameraCoordinateToCameraPixelAtRandomPosition) {
    CamCoord cc(1.44932752_m, -0.14675134_m, 24.60576372_m);
    CameraPixel cp = world_.ToCameraPixel(cc);
    EXPECT_NEAR((double)cp.x, 2472.18737, 1e-4f);
    EXPECT_NEAR((double)cp.y, 1627.05144, 1e-4f);
}

TEST_F(WorldTest, WorldCoordinateToCameraCoordinateAtRandomPosition) {
    WorldCoord wc(-7.41505047_m, -25.1405782_m, 0.82177583_m);
    CamCoord cc = world_.ToCamCoord(wc);
    EXPECT_NEAR((double)cc.x, 1.43102217, 1e-7f);
    EXPECT_NEAR((double)cc.y, -0.27251013, 1e-7f);
    EXPECT_NEAR((double)cc.z, 24.60576372, 1e-7f);
}

TEST_F(WorldTest, WorldCoordinateToCameraPixelAtRandomPosition) {
    WorldCoord wc(-7.41505047_m, -25.1405782_m, 0.82177583_m);
    CameraPixel cp = world_.ToCameraPixel(wc);
    EXPECT_NEAR((double)cp.x, 2470.06312, 1e-5f);
    EXPECT_NEAR((double)cp.y, 1612.52734, 1e-5f);
}

TEST_F(WorldTest, CameraCoordinateToWorldCoordinateAtRandomPosition) {
    CamCoord cc(1.43102217_m, -0.27251013_m, 24.60576372_m);
    WorldCoord wc = world_.ToWorldCoord(cc);
    EXPECT_NEAR((double)wc.x, -7.41505047, 1e-7f);
    EXPECT_NEAR((double)wc.y, -25.1405782, 1e-7f);
    EXPECT_NEAR((double)wc.z, 0.82177583, 1e-7f);
}

TEST_F(WorldTest, GetHorizon) {
    double roll(world_.GetCamera().GetRotation().w);
    CameraPixel left, right;
    std::tie(left, right) = world_.GetHorizon();
    EXPECT_LT(double(left.x), 0);
    EXPECT_LT(double(left.y), 3288 / 2);
    EXPECT_GT(double(right.x), 4608);
    EXPECT_LT(double(right.y), 3288 / 2);
    EXPECT_GT(left.y, right.y);
    auto horizon_angle = std::atan2(double(right.y - left.y), double(right.x - left.x));
    EXPECT_NEAR(-roll, horizon_angle, 1e-3);
}
*/
