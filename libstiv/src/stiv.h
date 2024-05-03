/* tiv - terminal image viewer - copyleft 2013-2015 - pancake */

#ifndef __STIV_H__
#define __STIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>  

    /// @brief The stiv context.
    typedef struct stivctx_t stivctx_t;

    typedef enum {
        STIV_MODE_ASCII,
        STIV_MODE_ANSI,
        STIV_MODE_GREY,
        STIV_MODE_256,
        STIV_MODE_RGB
    } stiv_mode;

    /// @brief Create a stiv context from a raw RGB24 buffer
    /// @param buffer The raw RGB24 buffer
    /// @param buffer_size The size of the buffer in bytes
    /// @param width The width of the image in pixels
    /// @param height The height of the image in pixels
    /// @param mode The mode to use for rendering
    /// @return The stiv context
    /// @note The buffer will be copied and managed internally.
    /// @note The image will not be displayed until `stiv_display` 
    ///       is called with the context.
    /// @warning This function will return NULL on failure.
    stivctx_t* stiv_create(const uint8_t* buffer, size_t buffer_size,
        uint32_t width, uint32_t height, stiv_mode mode);

    /// @brief Create a stiv context from a JPEG file
    /// @param jpeg_filepath The path to the JPEG file
    /// @param width The width of the image in pixels
    /// @param height The height of the image in pixels
    /// @param mode The mode to use for rendering
    /// @return The stiv context
    /// @note The image will not be displayed until `stiv_display` 
    ///       is called with the context.
    /// @warning This function will return NULL on failure.
    stivctx_t* stiv_from_jpeg(const char* jpeg_filepath, uint32_t width,
        uint32_t height, stiv_mode mode);

    /// @brief Release the resources associated within a stiv context
    /// @param ctx The stiv context
    /// @note If `!ctx`, this is a no-op.
    void stiv_free(stivctx_t* ctx);

    /// @brief Display the image associated with a stiv context
    /// @param ctx The stiv context
    /// @note If `!ctx`, this is a no-op.
    void stiv_display(stivctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif // __STIV_H__