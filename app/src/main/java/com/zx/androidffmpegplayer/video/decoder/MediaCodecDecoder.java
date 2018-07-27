package com.zx.androidffmpegplayer.video.decoder;

import android.graphics.SurfaceTexture;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.opengl.GLES20;
import android.os.Build;
import android.util.Log;
import android.view.Surface;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * User: ShaudXiao
 * Date: 2018-07-27
 * Time: 09:43
 * Company: zx
 * Description:
 * FIXME
 */

public class MediaCodecDecoder implements SurfaceTexture.OnFrameAvailableListener {
    private static final String TAG = "MediaCodecDecoder";
    private static final boolean m_verbose = false;
    private static final String VIDEO_MIME_TYPE = "video/avc";

    private static final int ERROR_OK = 0;
    private static final int ERROR_EOF = 1;
    private static final int ERROR_FAIL = 2;
    private static final int ERROR_UNUSUAL = 3;

    private MediaExtractor m_eExtractor = null;
    private int m_videoTrackIndex = -1;
    private MediaFormat mMediaFormat = null;
    private long mDuraion = 0;
    private boolean mExtractorInOriginalStaet = true;


    private SurfaceTexture m_surfaceTexture = null;
    private Surface m_surface = null;

    private MediaCodec.BufferInfo m_bufferInfo = null;

    private MediaCodec m_decoder = null;
    private boolean m_decoderStarted = false;
    ByteBuffer[] m_decoderInputBuffers = null;

    private Object m_frameSyncObject = new Object(); // guards m_frameAvailable
    private boolean m_frameAvailable = false;

    private long m_timestampOfLastDecodedFrame = Long.MIN_VALUE;
    private long m_timestampOfCurTexFrame = Long.MIN_VALUE;
    private boolean m_firstPlaybackTexFrameUnconsumed = false;
    private boolean m_inputBufferQueued = false;
    private int m_pendingInputFrameCount = 0;
    private boolean m_sawInputEOS = false;
    private boolean m_sawOutputEOS = false;

    public MediaCodecDecoder() {
        m_bufferInfo = new MediaCodec.BufferInfo();
    }

    public boolean OpenFile(String videoFilePath, int texId) {
        if (IsValid()) {
            return false;
        }

        try {
            m_eExtractor = new MediaExtractor();
            m_eExtractor.setDataSource(videoFilePath);
            mExtractorInOriginalStaet = true;
        } catch (Exception e) {
            e.printStackTrace();
            closeFile();
            return false;

        }

        int numTracks = m_eExtractor.getTrackCount();
        for (int i = 0; i < numTracks; i++) {
            MediaFormat format = m_eExtractor.getTrackFormat(i);
            String mime = format.getString(MediaFormat.KEY_MIME);
            if (mime.startsWith("video/")) {
                if (m_verbose) {
                    Log.d(TAG, "Extractor selected track " + i + " (" + mime + "): " + format);
                }
                m_videoTrackIndex = i;
                break;
            }
        }

        if (m_videoTrackIndex < 0) {
            closeFile();
            return false;
        }

        m_eExtractor.selectTrack(m_videoTrackIndex);
        ;
        mMediaFormat = m_eExtractor.getTrackFormat(m_videoTrackIndex);
        if (Build.VERSION.SDK_INT == 16) {
            // NOTE: some android 4.1 devices (such as samsung GT-I8552) will
            // crash in MediaCodec.configure
            // if we don't set MediaFormat.KEY_MAX_INPUT_SIZE.
            // Please refer to
            // http://stackoverflow.com/questions/22457623/surfacetextures-onframeavailable-method-always-called-too-late
            mMediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 0);
        }
        mDuraion = mMediaFormat.getLong(MediaFormat.KEY_DURATION);
        final String mime = mMediaFormat.getString(MediaFormat.KEY_MIME);
        if (m_verbose) {
            Log.d(TAG, "Selected video track " + m_videoTrackIndex + ", (" + mime + "): " + mMediaFormat
                    + ", duration(us): " + mDuraion);
        }

        // create surfaceTexture object
        try {
            m_surfaceTexture = new SurfaceTexture(texId);
            if (m_verbose) {
                Log.d(TAG, "Surface texture with texture (id=" + texId + ") has been created.");
            }
            m_surfaceTexture.setOnFrameAvailableListener(this);
            m_surface = new Surface(m_surfaceTexture);

        } catch (Exception e) {
            e.printStackTrace();
            closeFile();
            return false;
        }
        if (!setupDecoder(mime)) {
            closeFile();
            return false;
        }

        return true;

    }

    private void closeFile() {
        cleanUpDecoder();
        if (m_surface != null) {
            m_surface.release();
            m_surface = null;
        }

        if (m_surfaceTexture != null) {
            m_surfaceTexture.release();
            m_surfaceTexture = null;
        }

        if (m_eExtractor != null) {
            m_eExtractor.release();
            m_eExtractor = null;
            m_videoTrackIndex = -1;
            mMediaFormat = null;
            mDuraion = 0;
            mExtractorInOriginalStaet = true;
        }
    }

    private int seekVideoFrame(long timestamp, long tolerance) {
        if (m_verbose) {
            Log.d(TAG, "SeekVideoFrame() called, timestamp=" + timestamp + ", tolerance=" + tolerance);
        }

        if (!IsValid()) {
            return ERROR_EOF;
        }

        timestamp = Math.max(timestamp, 0);
        if (timestamp > mDuraion) {
            return ERROR_EOF;
        }

        if (m_timestampOfCurTexFrame != Long.MIN_VALUE
                && Math.abs(timestamp - m_timestampOfCurTexFrame) <= tolerance) {
            return ERROR_OK;
        }

        return seekInternal(timestamp, tolerance);

    }

    public int StartPlayback(long timestamp, long tolerance) {
        if (m_verbose)
            Log.d(TAG, "StartPlayback() called, timestamp=" + timestamp + ", tolerance=" + tolerance);

        if (!IsValid())
            return ERROR_EOF;

        timestamp = Math.max(timestamp, 0);
        if (timestamp >= mDuraion)
            return ERROR_EOF;

        if (timestamp == m_timestampOfCurTexFrame && m_timestampOfCurTexFrame == m_timestampOfLastDecodedFrame) {
            m_firstPlaybackTexFrameUnconsumed = true;
            return ERROR_OK;
        }

        final int ret = seekInternal(timestamp, tolerance);
        if (ret != ERROR_OK)
            return ret;

        m_firstPlaybackTexFrameUnconsumed = true;
        return ERROR_OK;
    }

    public int GetNextVideoFrameForPlayback() {
        if (!IsValid())
            return ERROR_EOF;

        if (!m_firstPlaybackTexFrameUnconsumed) {
            final int ret = decodeToFrame(Long.MIN_VALUE, 0);
            if (ret != ERROR_OK)
                return ret;
        } else {
            m_firstPlaybackTexFrameUnconsumed = false;
        }

        return ERROR_OK;
    }

    public long GetTimestampOfCurrentTextureFrame() {
        return m_timestampOfCurTexFrame;
    }

    public void GetTransformMatrixOfSurfaceTexture(float[] mat) {
        if (m_surfaceTexture != null)
            m_surfaceTexture.getTransformMatrix(mat);
    }

    private int seekInternal(long timestamp, long tolerance) {
        boolean skipSeekFile = false;
        if (m_timestampOfLastDecodedFrame != Long.MIN_VALUE && timestamp > m_timestampOfLastDecodedFrame
                && timestamp < m_timestampOfLastDecodedFrame + 1500000) {
            // In this case, we don't issue seek command to MediaExtractor,
            // since decode from here to find the expected frame may be faster
            skipSeekFile = true;
        } else if (mExtractorInOriginalStaet && timestamp < 1500000) {
            skipSeekFile = true;
        }

        if (!skipSeekFile) {
            try {
                // Seek media extractor the closest Intra frame whose timestamp
                // is litte than 'timestamp'
                m_eExtractor.seekTo(timestamp, MediaExtractor.SEEK_TO_PREVIOUS_SYNC);
                if (m_verbose) {
                    Log.d(TAG, "Seek to " + timestamp);
                }

                if (m_sawInputEOS || m_sawOutputEOS) {
                    // The video decoder has seen the input EOS or the output
                    // EOS,
                    // It is not possible for the decoder to decode any more
                    // video frame after the seek point.
                    // Now we have to re-create the video decoder
                    cleanUpDecoder();
                    final String mime = mMediaFormat.getString(MediaFormat.KEY_MIME);
                    if (!setupDecoder(mime)) {
                        return ERROR_FAIL;
                    }

                    if (m_verbose) {
                        Log.d(TAG, "Decoder has been recreated.");
                    }
                } else {
                    if (m_inputBufferQueued) {
                        // NOTE: it seems that MediaCodec in some android
                        // devices (such as Xiaomi 2014011)
                        // will run into trouble if we call MediaCodec.flush()
                        // without queued any buffer before
                        m_decoder.flush();
                        m_inputBufferQueued = false;
                        m_pendingInputFrameCount = 0;
                        if (m_verbose) {
                            Log.d(TAG, "Video decoder has been flushed.");
                        }
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, "" + e.getMessage());
                e.printStackTrace();
                return ERROR_FAIL;
            }
        }

        return decodeToFrame(timestamp, tolerance);
    }

    private int decodeToFrame(long timestamp, long tolerance) {
        try {
            return doDecodeToFrame(timestamp, tolerance);
        } catch (Exception e) {
            Log.e(TAG, "" + e.getMessage());
            e.printStackTrace();
            // It is better to cleanup the decoder to prevent further error
            cleanUpDecoder();
            return ERROR_FAIL;
        }
    }

    private int doDecodeToFrame(long timestamp, long tolerance) {
        final int inputBufferCount = m_decoderInputBuffers.length;
        final int pendingInputBufferThreshold = Math.max(inputBufferCount / 3, 2);
        final int TIMEOUT_USEC = 4000;
        int deadDecoderCounter = 0;

        while (!m_sawOutputEOS) {
            if (!m_sawInputEOS) {
                // Feed more data to the decoder
                final int inputBufIndex = m_decoder.dequeueInputBuffer(TIMEOUT_USEC);
                if (inputBufIndex >= 0) {
                    ByteBuffer inputBuf = m_decoderInputBuffers[inputBufIndex];
                    // Read the sample data into the ByteBuffer. This neither
                    // respects nor
                    // updates inputBuf's position, limit, etc.
                    final int chunkSize = m_eExtractor.readSampleData(inputBuf, 0);

                    if (m_verbose)
                        Log.d(TAG, "input packet length: " + chunkSize + " time stamp: " + m_eExtractor.getSampleTime());

                    if (chunkSize < 0) {
                        // End of stream -- send empty frame with EOS flag set.
                        m_decoder.queueInputBuffer(inputBufIndex, 0, 0, 0L, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                        m_sawInputEOS = true;
                        if (m_verbose)
                            Log.d(TAG, "Sent input EOS");
                    } else {
                        if (m_eExtractor.getSampleTrackIndex() != m_videoTrackIndex) {
                            Log.w(TAG, "WEIRD: got sample from track " + m_eExtractor.getSampleTrackIndex()
                                    + ", expected " + m_videoTrackIndex);
                        }

                        long presentationTimeUs = m_eExtractor.getSampleTime();

                        m_decoder.queueInputBuffer(inputBufIndex, 0, chunkSize, presentationTimeUs, 0);
                        if (m_verbose)
                            Log.d(TAG,
                                    "Submitted frame to decoder input buffer " + inputBufIndex + ", size=" + chunkSize);

                        m_inputBufferQueued = true;
                        ++m_pendingInputFrameCount;
                        if (m_verbose)
                            Log.d(TAG, "Pending input frame count increased: " + m_pendingInputFrameCount);

                        m_eExtractor.advance();
                        mExtractorInOriginalStaet = false;
                    }
                } else {
                    if (m_verbose)
                        Log.d(TAG, "Input buffer not available");
                }
            }

            // Determine the expiration time when dequeue output buffer
            int dequeueTimeoutUs;
            if (m_pendingInputFrameCount > pendingInputBufferThreshold || m_sawInputEOS) {
                dequeueTimeoutUs = TIMEOUT_USEC;
            } else {
                // NOTE: Too few input frames has been queued and the decoder
                // has not yet seen input EOS
                // wait dequeue for too long in this case is simply wasting
                // time.
                dequeueTimeoutUs = 0;
            }

            // Dequeue output buffer
            final int decoderStatus = m_decoder.dequeueOutputBuffer(m_bufferInfo, dequeueTimeoutUs);

            if (m_verbose)
                Log.d(TAG, "decoderStatus is " + decoderStatus);

            ++deadDecoderCounter;
            if (decoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {
                // No output available yet
                if (m_verbose) {
                    Log.d(TAG, "No output from decoder available");
                }
            } else if (decoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                // Not important for us, since we're using Surface
                if (m_verbose) {
                    Log.d(TAG, "Decoder output buffers changed");
                }
            } else if (decoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                MediaFormat newFormat = m_decoder.getOutputFormat();
                if (m_verbose) {
                    Log.d(TAG, "Decoder output format changed: " + newFormat);
                }
            } else if (decoderStatus < 0) {
                Log.e(TAG, "Unexpected result from decoder.dequeueOutputBuffer: " + decoderStatus);
                return ERROR_FAIL;
            } else { // decoderStatus >= 0
                if (m_verbose) {
                    Log.d(TAG, "Surface decoder given buffer " + decoderStatus + " (size=" + m_bufferInfo.size + ") "
                            + " (pts=" + m_bufferInfo.presentationTimeUs + ") ");
                }

                if ((m_bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    if (m_verbose) {
                        Log.d(TAG, "Output EOS");
                    }
                    m_sawOutputEOS = true;
                }

                // Render to texture?
                boolean doRender = false;

                // NOTE: We don't use m_bufferInfo.size != 0 to determine
                // whether we can render the decoded frame,
                // since some stupid android devices such as XIAOMI 2S, Xiaomi
                // 2014011 ... will always report zero-sized buffer
                // if we have configured the video decoder with a surface.
                // Now we will render the frame if the m_bufferInfo didn't carry
                // MediaCodec.BUFFER_FLAG_END_OF_STREAM flag.
                // NOTE: this method is a hack and we may lose the last video
                // frame
                // if the last video frame carry the
                // MediaCodec.BUFFER_FLAG_END_OF_STREAM flag.
                if (!m_sawOutputEOS) {
                    deadDecoderCounter = 0;

                    // Update timestamp of last decoded video frame
                    m_timestampOfLastDecodedFrame = m_bufferInfo.presentationTimeUs;
                    --m_pendingInputFrameCount;
                    if (m_verbose) {
                        Log.d(TAG, "Pending input frame count decreased: " + m_pendingInputFrameCount);
                    }

                    if (timestamp != Long.MIN_VALUE) {
                        doRender = (m_timestampOfLastDecodedFrame >= (timestamp - tolerance));
                    } else {
                        doRender = true;
                    }
                }

                // As soon as we call releaseOutputBuffer, the buffer will be
                // forwarded
                // to SurfaceTexture to convert to a texture. The API doesn't
                // guarantee
                // that the texture will be available before the call returns,
                // so we
                // need to wait for the onFrameAvailable callback to fire.
                m_decoder.releaseOutputBuffer(decoderStatus, doRender);
                if (doRender) {
                    if (m_verbose) {
                        Log.d(TAG, "Rendering decoded frame to surface texture.");
                    }

                    if (AwaitNewImage()) {
                        // NOTE: don't use m_surfaceTexture.getTimestamp() here,
                        // it is incorrect!
                        m_timestampOfCurTexFrame = m_bufferInfo.presentationTimeUs;
                        if (m_verbose) {
                            Log.d(TAG, "Surface texture updated, pts=" + m_timestampOfCurTexFrame);
                        }

                        return ERROR_OK;
                    } else {
                        Log.e(TAG, "Render decoded frame to surface texture failed!");
                        return ERROR_FAIL;
                    }
                }
            }

            if (deadDecoderCounter > 100) {
                Log.e(TAG, "We have tried two many times and can't decode a frame!");
                return ERROR_EOF;
            }
        } // while (!m_sawOutputEOS)

        return ERROR_EOF;
    }

    private boolean AwaitNewImage() {
        final int TIMEOUT_MS = 3000;

        synchronized (m_frameSyncObject) {
            while (!m_frameAvailable) {
                try {
                    // Wait for onFrameAvailable() to signal us. Use a timeout
                    // to avoid
                    // stalling the test if it doesn't arrive.
                    m_frameSyncObject.wait(TIMEOUT_MS);
                    if (!m_frameAvailable) {
                        // TODO: if "spurious wakeup", continue while loop
                        Log.e(TAG, "Frame wait timed out!");
                        return false;
                    }
                } catch (InterruptedException ie) {
                    // Shouldn't happen
                    Log.e(TAG, "" + ie.getMessage());
                    ie.printStackTrace();
                    return false;
                }
            }

            m_frameAvailable = false;
        }

        if (m_verbose) {
            final int glErr = GLES20.glGetError();
            if (glErr != GLES20.GL_NO_ERROR) {
                Log.e(TAG, "Before updateTexImage(): glError " + glErr);
            }
        }

        // Latch the data
        // m_surfaceTexture.updateTexImage();

        if (m_verbose) {
            Log.d(TAG, "frame is available, need updateTexImage");
        }

        return true;
    }

    public boolean CreateVideoDecoder(int width, int height, int texId, byte[] sps, int spsSize, byte[] pps,
                                      int ppsSize) {
        if (IsValid()) {
            Log.e(TAG, "You can't call InitDecoder() twice!");
            return false;
        }

        mMediaFormat = MediaFormat.createVideoFormat(VIDEO_MIME_TYPE, width, height);

        mMediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(sps));
        mMediaFormat.setByteBuffer("csd-1", ByteBuffer.wrap(pps));

        if (Build.VERSION.SDK_INT == 16) {
            // NOTE: some android 4.1 devices (such as samsung GT-I8552) will
            // crash in MediaCodec.configure
            // if we don't set MediaFormat.KEY_MAX_INPUT_SIZE.
            // Please refer to
            // http://stackoverflow.com/questions/22457623/surfacetextures-onframeavailable-method-always-called-too-late
            mMediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 0);
        }

        if (m_verbose) {
            Log.d(TAG, "MediaFormat is " + mMediaFormat);
        }

        //
        // Create SurfaceTexture and its wrapper Surface object
        //
        try {
            m_surfaceTexture = new SurfaceTexture(texId);
            if (m_verbose) {
                Log.d(TAG, "Surface texture with texture (id=" + texId + ") has been created.");
            }
            m_surfaceTexture.setOnFrameAvailableListener(this);
            m_surface = new Surface(m_surfaceTexture);
        } catch (Exception e) {
            Log.e(TAG, "" + e.getMessage());
            e.printStackTrace();
            return false;
        }

        if (!setupDecoder(VIDEO_MIME_TYPE)) {
            closeFile();
            return false;
        }

        return true;
    }

    public int DecodeFrame(byte[] frameData, int inputSize, long timeStamp) {
        if (null == m_decoderInputBuffers) {
            return 0;
        }
        boolean bUnusual = false;
        try {
            final int inputBufferCount = m_decoderInputBuffers.length;
            final int pendingInputBufferThreshold = Math.max(inputBufferCount / 3, 2);
            final int TIMEOUT_USEC = 4000;

            if (!m_sawInputEOS) {
                // Feed more data to the decoder
                final int inputBufIndex = m_decoder.dequeueInputBuffer(TIMEOUT_USEC);
                if (inputBufIndex >= 0) {
                    ByteBuffer inputBuf = m_decoderInputBuffers[inputBufIndex];

                    if (inputSize == 0) { // input EOF
                        // End of stream -- send empty frame with EOS flag set.
                        m_decoder.queueInputBuffer(inputBufIndex, 0, 0, 0L, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                        m_sawInputEOS = true;
                        if (m_verbose) {
                            Log.d(TAG, "Input EOS");
                        }
                    } else {
                        inputBuf.clear();
                        inputBuf.put(frameData, 0, inputSize);

                        m_decoder.queueInputBuffer(inputBufIndex, 0, inputSize, timeStamp, 0);

                        if (m_verbose) {
                            Log.d(TAG, "Submitted frame to decoder input buffer " + inputBufIndex + ", size=" + inputSize);
                        }
                        m_inputBufferQueued = true;
                        ++m_pendingInputFrameCount;
                        if (m_verbose) {
                            Log.d(TAG, "Pending input frame count increased: " + m_pendingInputFrameCount);
                        }
                    }
                } else {
                    bUnusual = true;

                    if (m_verbose)
                        Log.d(TAG, "Input buffer not available");
                }
            }

            // Determine the expiration time when dequeue output buffer
            int dequeueTimeoutUs;
            if (m_pendingInputFrameCount > pendingInputBufferThreshold || m_sawInputEOS) {
                dequeueTimeoutUs = TIMEOUT_USEC;
            } else {
                // NOTE: Too few input frames has been queued and the decoder has
                // not yet seen input EOS
                // wait dequeue for too long in this case is simply wasting time.
                dequeueTimeoutUs = 0;
            }

            // Dequeue output buffer
            final int decoderStatus = m_decoder.dequeueOutputBuffer(m_bufferInfo, dequeueTimeoutUs);

            if (decoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {
                // No output available yet
                if (m_verbose)
                    Log.d(TAG, "No output from decoder available");
            } else if (decoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                // Not important for us, since we're using Surface
                if (m_verbose)
                    Log.d(TAG, "Decoder output buffers changed");
            } else if (decoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                MediaFormat newFormat = m_decoder.getOutputFormat();
                if (m_verbose)
                    Log.d(TAG, "Decoder output format changed: " + newFormat);
            } else if (decoderStatus < 0) {
                Log.e(TAG, "Unexpected result from decoder.dequeueOutputBuffer: " + decoderStatus);
                return ERROR_FAIL;
            } else { // decoderStatus >= 0
                if (m_verbose) {
                    Log.d(TAG, "Surface decoder given buffer " + decoderStatus + " (size=" + m_bufferInfo.size + ") "
                            + " (pts=" + m_bufferInfo.presentationTimeUs + ") ");
                }

                if ((m_bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    if (m_verbose)
                        Log.d(TAG, "Output EOS");
                    m_sawOutputEOS = true;
                }

                // Render to texture?
                boolean doRender = false;

                // NOTE: We don't use m_bufferInfo.size != 0 to determine whether we
                // can render the decoded frame,
                // since some stupid android devices such as XIAOMI 2S, Xiaomi
                // 2014011 ... will always report zero-sized buffer
                // if we have configured the video decoder with a surface.
                // Now we will render the frame if the m_bufferInfo didn't carry
                // MediaCodec.BUFFER_FLAG_END_OF_STREAM flag.
                // NOTE: this method is a hack and we may lose the last video frame
                // if the last video frame carry the
                // MediaCodec.BUFFER_FLAG_END_OF_STREAM flag.
                if (!m_sawOutputEOS) {
                    // Update timestamp of last decoded video frame
                    m_timestampOfLastDecodedFrame = m_bufferInfo.presentationTimeUs;
                    --m_pendingInputFrameCount;
                    if (m_verbose)
                        Log.d(TAG, "Pending input frame count decreased: " + m_pendingInputFrameCount);

                    doRender = true;
                }

                // As soon as we call releaseOutputBuffer, the buffer will be
                // forwarded
                // to SurfaceTexture to convert to a texture. The API doesn't
                // guarantee
                // that the texture will be available before the call returns, so we
                // need to wait for the onFrameAvailable callback to fire.
                m_decoder.releaseOutputBuffer(decoderStatus, doRender);
                if (doRender) {
                    if (m_verbose)
                        Log.d(TAG, "Rendering decoded frame to surface texture.");

                    if (AwaitNewImage()) {
                        // NOTE: don't use m_surfaceTexture.getTimestamp() here, it
                        // is incorrect!
                        m_timestampOfCurTexFrame = m_bufferInfo.presentationTimeUs;
                        if (m_verbose)
                            Log.d(TAG, "Surface texture updated, pts=" + m_timestampOfCurTexFrame);

                        return ERROR_OK;
                    } else {
                        Log.e(TAG, "Render decoded frame to surface texture failed!");
                        return ERROR_FAIL;
                    }
                } else
                    return ERROR_EOF;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        if (bUnusual)
            return ERROR_UNUSUAL;
        else
            return ERROR_FAIL;
    }

    private boolean IsValid() {
        return m_decoder != null;
    }

    private boolean setupDecoder(String mine) {
        try {
            m_decoder = MediaCodec.createDecoderByType(mine);
            m_decoder.configure(mMediaFormat, m_surface, null, 0);
            m_decoder.start();
            ;
            m_decoderStarted = true;

            m_decoderInputBuffers = m_decoder.getInputBuffers();
            if (m_verbose) {
                final int inputBufferCount = m_decoderInputBuffers.length;
            }
        } catch (IOException e) {
            e.printStackTrace();
            cleanUpDecoder();
            return false;
        }
        return true;
    }

    public void cleanUpDecoder() {
        if (null != m_decoder) {
            if (m_decoderStarted) {
                try {
                    if (m_inputBufferQueued) {
                        m_decoder.flush();
                        m_inputBufferQueued = false;
                    }
                    m_decoder.stop();
                } catch (Exception e) {
                    e.printStackTrace();
                }
                m_decoderStarted = false;
                m_decoderInputBuffers = null;
            }
            m_decoder.release();
            m_decoder = null;
        }

        m_timestampOfLastDecodedFrame = Long.MIN_VALUE;
        m_timestampOfCurTexFrame = Long.MIN_VALUE;
        m_firstPlaybackTexFrameUnconsumed = false;
        m_pendingInputFrameCount = 0;
        m_sawInputEOS = false;
        m_sawOutputEOS = false;
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        synchronized (m_frameSyncObject) {
            if (m_frameAvailable) {
                Log.e(TAG, "m_frameAvailable already set, frame could be dropped!");
            }

            m_frameAvailable = true;
            m_frameSyncObject.notifyAll();
        }
    }

    public long updateTexImage() {
        if (m_verbose)
            Log.d(TAG, "before m_surfaceTexture.updateTexImage()" + m_timestampOfCurTexFrame);

        try {
            m_surfaceTexture.updateTexImage();
        } catch (Exception e) {
            e.printStackTrace();
        }

        if (m_verbose)
            Log.d(TAG, "after m_surfaceTexture.updateTexImage()");

        return m_timestampOfCurTexFrame;
    }

    public void beforeSeek() {
        if (m_sawInputEOS || m_sawOutputEOS) {
            // The video decoder has seen the input EOS or the output EOS,
            // It is not possible for the decoder to decode any more video frame
            // after the seek point.
            // Now we have to re-create the video decoder
            cleanUpDecoder();
            final String mime = mMediaFormat.getString(MediaFormat.KEY_MIME);
            if (!setupDecoder(mime))
                return;

            if (m_verbose)
                Log.d(TAG, "Decoder has been recreated.");
        } else {
            if (m_inputBufferQueued) {
                // NOTE: it seems that MediaCodec in some android devices (such
                // as Xiaomi 2014011)
                // will run into trouble if we call MediaCodec.flush() without
                // queued any buffer before
                m_decoder.flush();
                m_inputBufferQueued = false;
                m_pendingInputFrameCount = 0;
                if (m_verbose)
                    Log.d(TAG, "Video decoder has been flushed.");
            }
        }
    }


    public static boolean IsInAndriodHardwareBlacklist() {
        String manufacturer = Build.MANUFACTURER;
        String model = Build.MODEL;
        Log.i("problem", manufacturer);
        Log.i("problem", model);

        // too slow
        if ((manufacturer.compareTo("Meizu") == 0) && (model.compareTo("m2") == 0)) {
            return true;
        }

        if ((manufacturer.compareTo("Meizu") == 0) && (model.compareTo("M351") == 0)) {
            return true;
        }

        if ((manufacturer.compareTo("HUAWEI") == 0) && (model.compareTo("SUR-TL01H") == 0)) {
            return true;
        }

        if ((manufacturer.compareTo("Xiaomi") == 0) && (model.compareTo("MI 4W") == 0)) {
            return true;
        }
        //Mali-T720
        if ((manufacturer.compareTo("HUAWEI") == 0) && (model.compareTo("HUAWEI TAG-AL00") == 0)) {
            return true;
        }
        //Mali-T760
        if ((manufacturer.compareTo("samsung") == 0) && (model.compareTo("SM-G9250") == 0)) {
            return true;
        }
        //Vivante GC2000
        if ((manufacturer.compareTo("Coolpad") == 0) && (model.compareTo("Coolpad 8720L") == 0)) {
            return true;
        }
        //PowerVR SGX 544MP
        if ((manufacturer.compareTo("samsung") == 0) && (model.compareTo("GT-I9500") == 0)) {
            return true;
        }
        //Mali-450 MP
        if ((manufacturer.compareTo("BBK") == 0) && (model.compareTo("vivo X5L") == 0)) {
            return true;
        }

        return false;
    }

    public static boolean IsInAndriodHardwareWhitelist() {
        String manufacturer = Build.MANUFACTURER;
        String model = Build.MODEL;

        if ((manufacturer.compareTo("samsung") == 0) && (model.compareTo("GT-I9152") == 0)) {
            return true;
        }

        if ((manufacturer.compareTo("HUAWEI") == 0) && (model.compareTo("HUAWEI P6-C00") == 0)) {
            return true;
        }

        return false;
    }
}
