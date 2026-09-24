//
//  ORBPathCallbacks.cpp
//  OpenRenderBox
//
//  Audited for 6.5.1
//  Status: Complete

#if OPENRENDERBOX_CF_CGTYPES

#include <OpenRenderBox/ORBPathCallbacks.h>
#include <OpenRenderBoxCxx/Util/assert.hpp>

#if ORB_TARGET_OS_DARWIN

namespace {
    CGPathRef cgPathFromStorage(const void *object) {
        return reinterpret_cast<CGPathRef>(const_cast<void *>(object));
    }

    uint32_t cgpath_bezier_order(CGPathRef cgPath) {
        bool hasCurve = false;
        CGPathApply(cgPath, &hasCurve, [](void *info, const CGPathElement *element) {
            if (element->type == kCGPathElementAddQuadCurveToPoint
                || element->type == kCGPathElementAddCurveToPoint) {
                *static_cast<bool*>(info) = true;
            }
        });
        return hasCurve ? 3 : 1;
    }
}

const ORBPathCallbacks ORBPathCGPathCallbacks = {
    .flags = {},
    .retain = CFRetain,
    .release = CFRelease,
    .apply = +[](const void *object, void *info, ORBPathApplyCallback callback) -> bool {
        struct ApplyContext {
            void *info;
            ORBPathApplyCallback callback;
            bool shouldStop;
        } context = {info, callback, false};
        CGPathApply(cgPathFromStorage(object), &context, [](void *rawContext, const CGPathElement *element) {
            auto *context = static_cast<ApplyContext *>(rawContext);
            if (context->shouldStop) { return; }
            context->shouldStop = !context->callback(
                context->info,
                (ORBPathElement)element->type,
                (CGFloat *)element->points,
                nullptr
            );
        });
        return !context.shouldStop;
    },
    .isEqual = +[](const void *object, const void *otherObject) -> bool {
        return CGPathEqualToPath(cgPathFromStorage(object), cgPathFromStorage(otherObject));
    },
    .isEmpty = +[](const void *object) -> bool {
        return CGPathIsEmpty(cgPathFromStorage(object));
    },
    .isSingleElement = +[](const void *object) -> bool {
        return false;
    },
    .bezierOrder = +[](const void *object) -> uint32_t {
        return cgpath_bezier_order(cgPathFromStorage(object));
    },
    .boundingRect = +[](const void *object) -> CGRect {
        return CGPathGetPathBoundingBox(cgPathFromStorage(object));
    },
    .cgPath = +[](const void *object) -> CGPathRef {
        return cgPathFromStorage(object);
    },
    .next = nullptr,
};

#endif /* ORB_TARGET_OS_DARWIN */

#endif /* OPENRENDERBOX_CF_CGTYPES */
