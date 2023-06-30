// Copyright 2021-2023 UT-Battelle
// See LICENSE.txt in the root of the source distribution for license info.
#pragma once

#include "h4i/mklshim/Context.h"

namespace H4I::HIPUtils
{

template<typename HandleType,
            typename StatusType,
            StatusType successStatus,
            StatusType nullHandleStatus>
class HandleManager
{
private:
    struct StreamHandles
    {
        H4I::MKLShim::Context::NativeHandleArray handles;
        int nHandles;

        StreamHandles(hipStream_t stream = nullptr)
          : nHandles(handles.size())
        {    
            // Note this code uses a chipStar extension to the HIP API.
            // See chipStar documentation for its use.
            // Both Level Zero and OpenCL backends currently require us
            // to pass nHandles = 4, and provide space for at least 4 handles.
            // TODO is there a way to query this info at runtime?
            hipGetBackendNativeHandles(reinterpret_cast<uintptr_t>(stream),
                handles.data(), &nHandles);
        }
    };

public:
    StatusType Create(HandleType* handle)
    {
        if(handle != nullptr)
        {
            // Determine the backend we're using.
            auto backend = H4I::MKLShim::Context::ToBackend(hipGetBackendName());

            // Obtain the native backend handles.
            StreamHandles streamHandles;

            // Associate the backend handles with the given handle variable.
            *handle = H4I::MKLShim::Context::Create(streamHandles.handles, backend);
        }
        return (*handle != nullptr) ? successStatus : nullHandleStatus;
    }

    StatusType SetStream(HandleType handle, hipStream_t stream)
    {
        if(handle != nullptr)
        {
            // Access the real backend handle context associated with the given handle.
            H4I::MKLShim::Context* ctxt = static_cast<H4I::MKLShim::Context*>(handle);
            assert(ctxt != nullptr);

            // Obtain the native backend handles associated with the given stream.
            StreamHandles streamHandles(stream);

            // Associate the stream's native handle with our context.
            ctxt->SetStream(streamHandles.handles);
        }
        return (handle != nullptr) ? successStatus : nullHandleStatus;
    }

    StatusType Destroy(HandleType handle)
    {
        if(handle != nullptr)
        {
            H4I::MKLShim::Context* ctxt = static_cast<H4I::MKLShim::Context*>(handle);
            assert(ctxt != nullptr);
            delete ctxt;
        }
        return (handle != nullptr) ? successStatus : nullHandleStatus;
    }
};

} // namespace H4I::HIPUtils

