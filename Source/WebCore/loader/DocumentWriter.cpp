/*
 * Copyright (C) 2010. Adam Barth. All rights reserved.
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "DocumentWriter.h"

#include "ContentSecurityPolicy.h"
#include "DOMImplementation.h"
#include "DocumentInlines.h"
#include "DocumentLoader.h"
#include "FrameLoader.h"
#include "FrameLoaderStateMachine.h"
#include "HistoryController.h"
#include "HistoryItem.h"
#include "LocalDOMWindow.h"
#include "LocalFrame.h"
#include "LocalFrameLoaderClient.h"
#include "LocalFrameView.h"
#include "MIMETypeRegistry.h"
#include "Page.h"
#include "PluginDocument.h"
#include "RawDataDocumentParser.h"
#include "ScriptController.h"
#include "ScriptableDocumentParser.h"
#include "SecurityOrigin.h"
#include "SecurityOriginPolicy.h"
#include "SecurityPolicy.h"
#include "SegmentedString.h"
#include "Settings.h"
#include "SinkDocument.h"
#include "TextResourceDecoder.h"
#include <wtf/Ref.h>

namespace WebCore {

static inline bool canReferToParentFrameEncoding(const LocalFrame* frame, const LocalFrame* parentFrame) 
{
    if (is<XMLDocument>(frame->document()))
        return false;
    return parentFrame && parentFrame->document()->securityOrigin().isSameOriginDomain(frame->document()->securityOrigin());
}
    
// This is only called by ScriptController::executeIfJavaScriptURL
// and always contains the result of evaluating a javascript: url.
// This is the <iframe src="javascript:'html'"> case.
void DocumentWriter::replaceDocumentWithResultOfExecutingJavascriptURL(const String& source, Document* ownerDocument)
{
    Ref frame = *m_frame;
    frame->checkedLoader()->stopAllLoaders();

    // If we are in the midst of changing the frame's document, don't execute script
    // that modifies the document further:
    if (frame->documentIsBeingReplaced())
        return;

    begin(frame->document()->url(), true, ownerDocument);

    setEncoding("UTF-8"_s, IsEncodingUserChosen::No);

    // begin() might fire an unload event, which will result in a situation where no new document has been attached,
    // and the old document has been detached. Therefore, bail out if no document is attached.
    if (!frame->document())
        return;

    if (!source.isNull()) {
        if (!m_hasReceivedSomeData) {
            m_hasReceivedSomeData = true;
            frame->protectedDocument()->setCompatibilityMode(DocumentCompatibilityMode::NoQuirksMode);
        }

        if (RefPtr parser = frame->document()->parser()) {
            auto utf8Source = source.utf8();
            parser->appendBytes(*this, reinterpret_cast<const uint8_t*>(utf8Source.data()), utf8Source.length());
        }
    }

    end();
}

void DocumentWriter::clear()
{
    m_decoder = nullptr;
    m_hasReceivedSomeData = false;
    if (!m_encodingWasChosenByUser)
        m_encoding = String();
}

bool DocumentWriter::begin()
{
    return begin(URL());
}

Ref<Document> DocumentWriter::createDocument(const URL& url, ScriptExecutionContextIdentifier documentIdentifier)
{
    printf("DW::createDocumennt() (1)\n");
    Ref frame = *m_frame;
    CheckedRef frameLoader = frame->loader();
    printf("DW::createDocumennt() (2)\n");
    if (!frameLoader->stateMachine().isDisplayingInitialEmptyDocument() && frameLoader->client().shouldAlwaysUsePluginDocument(m_mimeType))
        return PluginDocument::create(frame, url);

    auto useSinkDocument = [&]() {
    printf("DW::createDocumennt() (3)\n");
#if ENABLE(PDF_PLUGIN)
        if (frameLoader->client().shouldUsePDFPlugin(m_mimeType, url.path()))
            return false;
#endif
    printf("DW::createDocumennt() (4)\n");
#if PLATFORM(IOS_FAMILY)
        if (frame->isMainFrame())
            return true;

    printf("DW::createDocumennt() (5)\n");
        return !frame->settings().useImageDocumentForSubframePDF();
#else
    printf("DW::createDocumennt() (6)\n");
        return false;
#endif
    };

    printf("DW::createDocumennt() (7)\n");
    if (MIMETypeRegistry::isPDFMIMEType(m_mimeType) && useSinkDocument())
        return SinkDocument::create(frame, url);

    printf("DW::createDocumennt() (8)\n");
    if (!frameLoader->client().hasHTMLView())
        return Document::createNonRenderedPlaceholder(frame, url);

    printf("DW::createDocumennt() (9)\n");
    auto result = DOMImplementation::createDocument(m_mimeType, frame.ptr(), frame->settings(), url, documentIdentifier);
    printf("Returned from DOMI::createDocument()\n");
    return result;
}

bool DocumentWriter::begin(const URL& urlReference, bool dispatch, Document* ownerDocument, ScriptExecutionContextIdentifier documentIdentifier, const NavigationAction* triggeringAction)
{
    printf("begin() (1)\n");
    // We grab a local copy of the URL because it's easy for callers to supply
    // a URL that will be deallocated during the execution of this function.
    // For example, see <https://bugs.webkit.org/show_bug.cgi?id=66360>.
    URL url = urlReference;

    // Create a new document before clearing the frame, because it may need to
    // inherit an aliased security context.
    Ref document = createDocument(url, documentIdentifier);
    
    printf("begin() (2)\n");
    Ref frame = *m_frame;
    CheckedRef frameLoader = frame->loader();
    printf("begin() (3)\n");

    // If the new document is for a Plugin but we're supposed to be sandboxed from Plugins,
    // then replace the document with one whose parser will ignore the incoming data (bug 39323)
    if (document->isPluginDocument() && document->isSandboxed(SandboxPlugins))
        document = SinkDocument::create(frame, url);

    // FIXME: Do we need to consult the content security policy here about blocked plug-ins?

    printf("begin() (4)\n");
    bool shouldReuseDefaultView = frameLoader->stateMachine().isDisplayingInitialEmptyDocument()
        && frame->document()->isSecureTransitionTo(url)
        && (frame->window() && !frame->window()->wasWrappedWithoutInitializedSecurityOrigin() && frame->window()->mayReuseForNavigation());

    printf("begin() (5)\n");
    if (shouldReuseDefaultView) {
    printf("begin() (6)\n");
        ASSERT(frameLoader->documentLoader());
        if (CheckedPtr contentSecurityPolicy = frameLoader->documentLoader()->contentSecurityPolicy())
            shouldReuseDefaultView = !(contentSecurityPolicy->sandboxFlags() & SandboxOrigin);
    printf("begin() (7)\n");
    }

    // Temporarily extend the lifetime of the existing document so that FrameLoader::clear() doesn't destroy it as
    // we need to retain its ongoing set of upgraded requests in new navigation contexts per <http://www.w3.org/TR/upgrade-insecure-requests/>
    // and we may also need to inherit its Content Security Policy below.
    printf("begin() (8)\n");
    RefPtr existingDocument = frame->document();
    printf("begin() (9)\n");

    Function<void()> handleDOMWindowCreation = [document, frame, shouldReuseDefaultView] {
    printf("begin() (10)\n");
        if (shouldReuseDefaultView)
            document->takeDOMWindowFrom(*frame->protectedDocument());
        else
            document->createDOMWindow();
    printf("begin() (11)\n");
    };
    printf("begin() (12)\n");

    frameLoader->clear(document.ptr(), !shouldReuseDefaultView, !shouldReuseDefaultView, true, WTFMove(handleDOMWindowCreation));
    printf("begin() (13)\n");
    clear();
    printf("begin() (14)\n");

    // frameLoader->clear() might fire unload event which could remove the view of the document.
    // Bail out if document has no view.
    if (!document->view())
        return false;
    printf("begin() (15)\n");

    if (!shouldReuseDefaultView)
        frame->checkedScript()->updatePlatformScriptObjects();
    printf("begin() (16)\n");

    frameLoader->setOutgoingReferrer(url);
    printf("begin() (17)\n");
    frame->setDocument(document.copyRef());
    printf("begin() (18)\n");

    if (RefPtr decoder = m_decoder)
        document->setDecoder(decoder.get());
    printf("begin() (19)\n");
    if (ownerDocument) {
    printf("begin() (20)\n");
        // |document| is the result of evaluating a JavaScript URL.
        document->setCookieURL(ownerDocument->cookieURL());
    printf("begin() (21)\n");
        document->setSecurityOriginPolicy(ownerDocument->securityOriginPolicy());
    printf("begin() (22)\n");
        document->setStrictMixedContentMode(ownerDocument->isStrictMixedContentMode());
    printf("begin() (23)\n");
        document->setCrossOriginEmbedderPolicy(ownerDocument->crossOriginEmbedderPolicy());
    printf("begin() (24)\n");

        document->setContentSecurityPolicy(makeUnique<ContentSecurityPolicy>(URL { url }, document));
    printf("begin() (25)\n");
        CheckedRef contentSecurityPolicy = *document->contentSecurityPolicy();
    printf("begin() (26)\n");
        CheckedRef ownerContentSecurityPolicy = *ownerDocument->contentSecurityPolicy();
    printf("begin() (27)\n");
        contentSecurityPolicy->copyStateFrom(ownerContentSecurityPolicy.ptr());
    printf("begin() (28)\n");
        contentSecurityPolicy->setInsecureNavigationRequestsToUpgrade(ownerContentSecurityPolicy->takeNavigationRequestsToUpgrade());
    printf("begin() (29)\n");
    } else if (url.protocolIsAbout() || url.protocolIsData()) {
    printf("begin() (30)\n");
        // https://html.spec.whatwg.org/multipage/origin.html#determining-navigation-params-policy-container
        RefPtr currentHistoryItem = frameLoader->history().currentItem();
    printf("begin() (31)\n");

        if (currentHistoryItem && currentHistoryItem->policyContainer()) {
    printf("begin() (32)\n");
            const auto& policyContainerFromHistory = currentHistoryItem->policyContainer();
            ASSERT(policyContainerFromHistory);
    printf("begin() (33)\n");
            document->inheritPolicyContainerFrom(*policyContainerFromHistory);
    printf("begin() (34)\n");
        } else if (url == aboutSrcDocURL()) {
    printf("begin() (35)\n");
            RefPtr parentFrame = dynamicDowncast<LocalFrame>(frame->tree().parent());
    printf("begin() (36)\n");
            if (parentFrame && parentFrame->document()) {
    printf("begin() (37)\n");
                document->inheritPolicyContainerFrom(parentFrame->document()->policyContainer());
    printf("begin() (38)\n");
                document->checkedContentSecurityPolicy()->updateSourceSelf(parentFrame->document()->securityOrigin());
    printf("begin() (39)\n");
            }
    printf("begin() (40)\n");
        } else if (triggeringAction && triggeringAction->requester()) {
    printf("begin() (41)\n");
            document->inheritPolicyContainerFrom(triggeringAction->requester()->policyContainer);
    printf("begin() (42)\n");
            document->checkedContentSecurityPolicy()->updateSourceSelf(triggeringAction->requester()->securityOrigin);
    printf("begin() (43)\n");
        }
    printf("begin() (44)\n");

        // https://html.spec.whatwg.org/multipage/origin.html#requires-storing-the-policy-container-in-history
        if (triggeringAction && triggeringAction->type() != NavigationType::BackForward && currentHistoryItem)
            currentHistoryItem->setPolicyContainer(document->policyContainer());
    printf("begin() (45)\n");
    }
    printf("begin() (46)\n");

    if (existingDocument && existingDocument->contentSecurityPolicy() && document->contentSecurityPolicy())
        document->checkedContentSecurityPolicy()->setInsecureNavigationRequestsToUpgrade(existingDocument->checkedContentSecurityPolicy()->takeNavigationRequestsToUpgrade());
    printf("begin() (47)\n");

    frameLoader->didBeginDocument(dispatch);
    printf("begin() (48)\n");

    document->implicitOpen();
    printf("begin() (49)\n");

    // We grab a reference to the parser so that we'll always send data to the
    // original parser, even if the document acquires a new parser (e.g., via
    // document.open).
    m_parser = document->parser();
    printf("begin() (50)\n");

    if (frame->view() && frameLoader->client().hasHTMLView())
        frame->protectedView()->setContentsSize(IntSize());
    printf("begin() (51)\n");

    m_state = State::Started;
    printf("begin() (52)\n");
    return true;
}

TextResourceDecoder& DocumentWriter::decoder()
{
    if (!m_decoder) {
        Ref frame = *m_frame;
        Ref decoder = TextResourceDecoder::create(m_mimeType, frame->settings().defaultTextEncodingName(), frame->settings().usesEncodingDetector());
        m_decoder = decoder.copyRef();
        RefPtr parentFrame = dynamicDowncast<LocalFrame>(frame->tree().parent());
        // Set the hint encoding to the parent frame encoding only if
        // the parent and the current frames share the security origin.
        // We impose this condition because somebody can make a child frame
        // containing a carefully crafted html/javascript in one encoding
        // that can be mistaken for hintEncoding (or related encoding) by
        // an auto detector. When interpreted in the latter, it could be
        // an attack vector.
        // FIXME: This might be too cautious for non-7bit-encodings and
        // we may consider relaxing this later after testing.
        if (canReferToParentFrameEncoding(frame.ptr(), parentFrame.get()))
            decoder->setHintEncoding(parentFrame->document()->protectedDecoder().get());
        if (m_encoding.isEmpty()) {
            if (canReferToParentFrameEncoding(frame.ptr(), parentFrame.get()))
                decoder->setEncoding(parentFrame->document()->textEncoding(), TextResourceDecoder::EncodingFromParentFrame);
        } else {
            decoder->setEncoding(m_encoding,
                m_encodingWasChosenByUser ? TextResourceDecoder::UserChosenEncoding : TextResourceDecoder::EncodingFromHTTPHeader);
        }
        frame->protectedDocument()->setDecoder(WTFMove(decoder));
    }
    return *m_decoder;
}

void DocumentWriter::reportDataReceived()
{
    ASSERT(m_decoder);
    if (m_hasReceivedSomeData)
        return;
    m_hasReceivedSomeData = true;
    Ref document = *m_frame->document();
    if (m_decoder->encoding().usesVisualOrdering())
        document->setVisuallyOrdered();
    document->resolveStyle(Document::ResolveStyleType::Rebuild);
}

RefPtr<DocumentParser> DocumentWriter::protectedParser() const
{
    return m_parser;
}

void DocumentWriter::addData(const SharedBuffer& data)
{
    // FIXME: Change these to ASSERT once https://bugs.webkit.org/show_bug.cgi?id=80427 has been resolved.
    RELEASE_ASSERT(m_state != State::NotStarted);
    if (m_state == State::Finished) {
        ASSERT_NOT_REACHED();
        return;
    }
    ASSERT(m_parser);
    protectedParser()->appendBytes(*this, data.data(), data.size());
}

void DocumentWriter::insertDataSynchronously(const String& markup)
{
    ASSERT(m_state != State::NotStarted);
    ASSERT(m_state != State::Finished);
    ASSERT(m_parser);
    protectedParser()->insert(markup);
}

void DocumentWriter::end()
{
    ASSERT(m_frame->page());
    ASSERT(m_frame->document());

    // The parser is guaranteed to be released after this point. begin() would
    // have to be called again before we can start writing more data.
    m_state = State::Finished;

    // http://bugs.webkit.org/show_bug.cgi?id=10854
    // The frame's last ref may be removed and it can be deleted by checkCompleted(), 
    // so we'll add a protective refcount
    Ref protectedFrame { *m_frame };

    if (!m_parser)
        return;
    // FIXME: m_parser->finish() should imply m_parser->flush().
    protectedParser()->flush(*this);
    if (!m_parser)
        return;
    protectedParser()->finish();
    m_parser = nullptr;
}

void DocumentWriter::setEncoding(const String& name, IsEncodingUserChosen isUserChosen)
{
    m_encoding = name;
    m_encodingWasChosenByUser = isUserChosen == IsEncodingUserChosen::Yes;
}

void DocumentWriter::setFrame(LocalFrame& frame)
{
    m_frame = frame;
}

void DocumentWriter::setDocumentWasLoadedAsPartOfNavigation()
{
    ASSERT(m_parser && !m_parser->isStopped());
    protectedParser()->setDocumentWasLoadedAsPartOfNavigation();
}

} // namespace WebCore
