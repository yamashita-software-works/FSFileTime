PostBuildSteps: $(TARGET)
	md ..\lib\$(_BUILDARCH)\lib$(_BUILDTYPE)
	copy obj$(BUILD_ALT_DIR)\$(TARGET_DIRECTORY)\$(TARGETNAME).lib ..\lib\$(_BUILDARCH)\lib$(_BUILDTYPE)