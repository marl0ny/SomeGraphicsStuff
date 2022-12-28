import Foundation
import MetalKit

typealias float2 = SIMD2<Float>
typealias float3 = SIMD3<Float>
typealias float4 = SIMD4<Float>


struct RenderFrame {
    var vertexBuffer: MTLBuffer
    var textureDescriptor: MTLTextureDescriptor
    var texture: MTLTexture
    var renderPassDescriptor: MTLRenderPassDescriptor
}


class GView: MTKView {
    
    var queue: MTLCommandQueue?
    var renderPipelineState: MTLRenderPipelineState?
    var renderPipelineState2: MTLRenderPipelineState?
    var renderPipelineStateWave: MTLRenderPipelineState?
    var renderPipelineRGBAdd2: MTLRenderPipelineState?
    var renderPipelineCopy: MTLRenderPipelineState?
    var renderPipelineZero: MTLRenderPipelineState?
    let QUAD_VERTEX_ARRAY: [float3] = [float3(-1.0, 1.0, 0.0), float3(1.0, 1.0, 0.0),
                                       float3(1.0, -1.0, 0.0), float3(1.0, -1.0, 0.0),
                                       float3(-1.0, -1.0, 0.0), float3(-1.0, 1.0, 0.0)]
    let QUAD_PRIMATIVE_TYPE: MTLPrimitiveType  = .triangle
    let QUAD_NUMBER_OF_VERTICES: Int = 6
    var renderFrames: [RenderFrame?] = [];
    var drawFrame: RenderFrame?
    let vertexArray: [float3] = [float3(-1.0, 1.0, 0.0), float3(1.0, 1.0, 0.0),
                                 float3(1.0, -1.0, 0.0), float3(1.0, -1.0, 0.0),
                                 float3(-1.0, -1.0, 0.0), float3(-1.0, 1.0, 0.0)]
    var vertexBuffer: MTLBuffer?
    var textureDescriptor: MTLTextureDescriptor?
    var texture: MTLTexture?
    var initStep: Bool = true
    var mouseChanged: Bool = false
    var clearAll: Bool = false
    var mouseLocation: NSPoint = NSPoint(x: 512, y: 512)
    override var acceptsFirstResponder: Bool {return true}
    
    
    required init(coder: NSCoder) {
        super.init(coder: coder)
        // self.window?.aspectRatio = NSSize(width: 4, height: 3)
        self.device = MTLCreateSystemDefaultDevice()
        self.clearColor = MTLClearColor(
            red: 1.0, green: 1.0, blue: 1.0, alpha: 1.0)
        self.colorPixelFormat = .bgra8Unorm
        self.queue = device?.makeCommandQueue()
        self.renderPipelineState = makeRenderPipelineState(vertexFunc: "vertex_func", fragmentFunc: "main_fragment_view", pixelFormat: self.colorPixelFormat)
        self.renderPipelineState2 = makeRenderPipelineState(vertexFunc: "vertex_func", fragmentFunc: "gaussian", pixelFormat: .rgba32Float)
        self.renderPipelineStateWave = makeRenderPipelineState(vertexFunc: "vertex_func", fragmentFunc: "wave_step", pixelFormat: .rgba32Float)
        self.renderPipelineRGBAdd2 = makeRenderPipelineState(vertexFunc: "vertex_func", fragmentFunc: "add2_rgb", pixelFormat: .rgba32Float)
        self.renderPipelineCopy = makeRenderPipelineState(vertexFunc: "vertex_func", fragmentFunc: "copy", pixelFormat: .rgba32Float)
        self.renderPipelineZero = makeRenderPipelineState(vertexFunc: "vertex_func", fragmentFunc: "zero", pixelFormat: .rgba32Float)
        // Similar to making a vao?
        self.vertexBuffer = device!.makeBuffer(bytes: vertexArray, length: vertexArray.count*MemoryLayout<float3>.stride, options: [])
        var arr: [Float32] = Array.init(repeating: 1.0, count: 4*512*512)
        for  i in 0..<512*512 {
            for (j, e) in [0.0, 1.0, 0.0, 1.0].enumerated() {
                let index = 4*i + j
                arr[index] = Float32(e)
            }
        }
        self.texture = makeTexture(width: 512, height: 512, pixelFormat: .rgba32Float)
        self.drawFrame = makeRenderFrame(width: 512, height: 512)
        for _ in 0..<3 {
            self.renderFrames.append(makeRenderFrame(width: 512, height: 512))
        }
    }
    
    
    override func keyDown(with event: NSEvent) {
        // g = 5, z = 6, etc
        if (event.keyCode == 5) {
            clearAll = true
        }
    }
    
    override func mouseDown(with event: NSEvent) {
        self.mouseChanged = true
        self.mouseLocation = event.locationInWindow
    }
        
    func makeRenderFrame(width: Int, height: Int, usage: MTLTextureUsage = .unknown,
                         pixelFormat: MTLPixelFormat = .rgba32Float,
                         inVertexBuffer: MTLBuffer? = nil) -> RenderFrame? {
        guard let device = self.device else {
            print("Unable to make device.")
            return nil
        }
        let vertexBuffer: MTLBuffer? = inVertexBuffer ?? device.makeBuffer(bytes: QUAD_VERTEX_ARRAY, length: QUAD_VERTEX_ARRAY.count*MemoryLayout<float3>.stride, options: [])
        guard vertexBuffer != nil else {
            print("Unable to create vertex buffer.")
            return nil
        }
        let texDescriptor: MTLTextureDescriptor = MTLTextureDescriptor()
        texDescriptor.pixelFormat = pixelFormat
        texDescriptor.width = width
        texDescriptor.height = height
        texDescriptor.usage = usage
        guard let texture = device.makeTexture(descriptor: texDescriptor) else {
            print("Unable to make texture")
            return nil
        }
        let renderPassDescriptor = MTLRenderPassDescriptor()
        renderPassDescriptor.colorAttachments[0].texture = texture
        renderPassDescriptor.colorAttachments[0].loadAction = .clear
        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 1.0)
        renderPassDescriptor.colorAttachments[0].storeAction = .store
        return  RenderFrame(vertexBuffer: vertexBuffer!, textureDescriptor: texDescriptor, texture: texture, renderPassDescriptor: renderPassDescriptor)
    }
    
    func makeQuad(width: Int, height: Int) -> RenderFrame? {
        return makeRenderFrame(width: width, height: height)
    }
    
    func drawQuad(commandBuffer: MTLCommandBuffer, frame: RenderFrame,
                  pipelineState: MTLRenderPipelineState, renderUseFunc: (MTLRenderCommandEncoder) -> Void) {
        let renderPassDescriptor = frame.renderPassDescriptor
        guard let renderCommandEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor) else {
            print("Unable to create render command encoder")
            return
        }
        renderCommandEncoder.setRenderPipelineState(pipelineState)
        renderUseFunc(renderCommandEncoder);
        renderCommandEncoder.setVertexBuffer(frame.vertexBuffer, offset: 0, index: 0)
        renderCommandEncoder.drawPrimitives(type: self.QUAD_PRIMATIVE_TYPE, vertexStart: 0, vertexCount: self.QUAD_NUMBER_OF_VERTICES)
        renderCommandEncoder.endEncoding()
    }
    
    private func makeRenderPipelineDescriptor(vertexFunc: String, fragmentFunc: String, pixelFormat: MTLPixelFormat) -> MTLRenderPipelineDescriptor {
        let library = device?.makeDefaultLibrary()
        let vertexFunc = library?.makeFunction(name: vertexFunc)
        let fragmentFunc = library?.makeFunction(name: fragmentFunc)
        let renderPipelineDiscriptor = MTLRenderPipelineDescriptor()
        renderPipelineDiscriptor.colorAttachments[0].pixelFormat = pixelFormat
        renderPipelineDiscriptor.vertexFunction = vertexFunc
        renderPipelineDiscriptor.fragmentFunction = fragmentFunc
        return renderPipelineDiscriptor
    }
    
    func makeRenderPipelineState(vertexFunc: String, fragmentFunc: String, pixelFormat: MTLPixelFormat) -> MTLRenderPipelineState? {
        let renderPipelineDescriptor = makeRenderPipelineDescriptor(vertexFunc: vertexFunc, fragmentFunc: fragmentFunc, pixelFormat: pixelFormat)
        let renderPipelineState: MTLRenderPipelineState
        do {
            renderPipelineState =
            try device!.makeRenderPipelineState(descriptor: renderPipelineDescriptor)
        } catch let exception as NSError {
            print(exception)
            return nil
        }
        return renderPipelineState
    }
    
    func makeTexture(width: Int, height: Int, pixelFormat: MTLPixelFormat,
                     usage: MTLTextureUsage = .unknown,
                     xOrigin: Int = 0, yOrigin: Int = 0, zOrigin: Int = 0,
                     depth: Int = 0) -> MTLTexture? {
        self.textureDescriptor = MTLTextureDescriptor()
        self.textureDescriptor?.pixelFormat = pixelFormat
        self.textureDescriptor?.width = width
        self.textureDescriptor?.height = height
        // self.textureDescriptor?.usage = .renderTarget // Error!
        // self.textureDescriptor?.usage = .shaderRead
        self.textureDescriptor?.usage = usage
        let texture: MTLTexture? = device?.makeTexture(descriptor: self.textureDescriptor!)
        return texture
    }
    
    func putDataInTexture(texture: inout MTLTexture, arr: inout [Float32]) {
        let width = texture.width, height = texture.height
        let pixelFormat = texture.pixelFormat
        if (pixelFormat != .rgba32Float) { return }
        if (width == 0 || height == 0) { return }
        let bytesPerRow = 16*width
        var region: MTLRegion = MTLRegion()
        region.origin = MTLOrigin(x: 0, y: 0, z: 0)
        region.size = MTLSize(width: width, height: height, depth: texture.depth)
        let arrPointer = UnsafeMutablePointer<Float32>.allocate(capacity: width*bytesPerRow)
        arrPointer.initialize(from: &arr, count:(bytesPerRow/4)*width)
        texture.replace(region: region, mipmapLevel: 0, withBytes: arrPointer, bytesPerRow: bytesPerRow)
        
    }
    
    private func makeComputePipelineState() -> MTLComputePipelineState? {
        let library = device?.makeDefaultLibrary()
        let fragmentFunc2 = library?.makeFunction(name: "fragment_func2")
        var computePipelineState: MTLComputePipelineState?
        do {
            try computePipelineState = device?.makeComputePipelineState(function: fragmentFunc2!)
        } catch let exception {
            print(exception)
        }
        return computePipelineState
    }
    
    
    override func draw(_ rect: NSRect) {
        
        guard let drawable = self.currentDrawable, let renderPassDiscriptor = self.currentRenderPassDescriptor, let queue = self.queue  else {
            return
        }
        let commandBuffer = queue.makeCommandBuffer()
        let randVal: Int = Int.random(in: 0..<3)
        let indices: [Int] = [0, 1, 2]
        let col: [Float] = indices.map{(e: Int) -> Float in e == randVal ? 1.0: 0.5}
        let gaussianUniformsFunc: (MTLRenderCommandEncoder) -> Void = {
            (encoder) in
            let offsetX: CGFloat = (rect.width/rect.height - 1.0)/2.0;
            let offsetY: CGFloat = (rect.height/rect.width - 1.0)/2.0;
            var array: [float4] = [];
            if rect.width > rect.height {
                array = [float4(Float(-offsetX + CGFloat(self.mouseLocation.x)/rect.height), Float((CGFloat(rect.height) - CGFloat(self.mouseLocation.y))/rect.height), 0.0, 0.0), float4(col[0], col[1], col[2], 1.0)];
            } else if rect.height > rect.width {
                array = [float4(Float(CGFloat(self.mouseLocation.x)/rect.width), Float(rect.height/rect.width - offsetY - CGFloat(self.mouseLocation.y/rect.width)), 0.0, 0.0), float4(col[0], col[1], col[2], 1.0)];
            } else {
                array = [float4(Float(CGFloat(self.mouseLocation.x)/rect.width), Float((CGFloat(rect.height) - CGFloat(self.mouseLocation.y))/rect.height), 0.0, 0.0), float4(col[0], col[1], col[2], 1.0)];
            }
            let gaussiansUniformBuffer: MTLBuffer = self.device!.makeBuffer(bytes: array, length: array.count*MemoryLayout<float4>.stride, options: [])!
            encoder.setFragmentBuffer(gaussiansUniformBuffer, offset: 0, index: 1)
        }
        
        if (self.clearAll) {
            for frame in self.renderFrames {
                drawQuad(commandBuffer: commandBuffer!, frame: frame!, pipelineState: self.renderPipelineZero!) {_ in}
            }
            self.clearAll = false
        }
        
        if (self.initStep) {
            drawQuad(commandBuffer: commandBuffer!, frame: self.renderFrames[0]!,    pipelineState: self.renderPipelineState2!, renderUseFunc: gaussianUniformsFunc)
            drawQuad(commandBuffer: commandBuffer!, frame: self.renderFrames[1]!,    pipelineState: self.renderPipelineState2!, renderUseFunc: gaussianUniformsFunc)
            drawQuad(commandBuffer: commandBuffer!, frame: self.renderFrames[2]!,    pipelineState: self.renderPipelineState2!, renderUseFunc: gaussianUniformsFunc)
            self.initStep = false
        } else {
            
            if (self.mouseChanged) {
                
                // draw to drawFrame
                drawQuad(commandBuffer: commandBuffer!, frame: self.drawFrame!, pipelineState: self.renderPipelineState2!, renderUseFunc: gaussianUniformsFunc)
                
                for i in 0...1 {
                    // Add the contents of renderFrames[i] and the drawFrame to
                    // renderFrames[2], then copy this back to renderFrames[i]
                    drawQuad(commandBuffer: commandBuffer!, frame: self.renderFrames[2]!, pipelineState: self.renderPipelineRGBAdd2!) {
                        (encoder) in
                        encoder.setFragmentTexture(self.drawFrame!.texture, index: 1)
                        encoder.setFragmentTexture(self.renderFrames[i]!.texture, index: 2)
                    }
                    drawQuad(commandBuffer: commandBuffer!, frame: self.renderFrames[i]!,
                             pipelineState: self.renderPipelineCopy!) {
                        (encoder) in
                        encoder.setFragmentTexture(self.renderFrames[2]!.texture, index: 0)
                    }
                    
                }
                self.mouseChanged = false
            }
            
            for _ in 1...3 {
                drawQuad(commandBuffer: commandBuffer!, frame: self.renderFrames[2]!,    pipelineState: self.renderPipelineStateWave!) {
                    (encoder) in
                    encoder.setFragmentTexture(self.renderFrames[0]!.texture, index: 0);
                    encoder.setFragmentTexture(self.renderFrames[1]!.texture, index: 1);
                }
                
                self.renderFrames = [self.renderFrames[1], self.renderFrames[2], self.renderFrames[0]]
            }
        }
        
        
        let frame: RenderFrame = self.renderFrames[2]!;
        let renderCommandEncoder = commandBuffer!.makeRenderCommandEncoder(descriptor: renderPassDiscriptor)
        
        let arr = [float2(Float(rect.width), Float(rect.height))]
        let uniformBuffer: MTLBuffer = self.device!.makeBuffer(bytes: arr, length: arr.count*MemoryLayout<float2>.stride, options: [])!
        
        // Set the render pipeline state that consists of the vertex and fragment shaders
        renderCommandEncoder?.setRenderPipelineState(self.renderPipelineState!)
        // Set the texture
        renderCommandEncoder?.setFragmentTexture(frame.texture, index: 2)
        // Similar to binding a vao?
        renderCommandEncoder?.setVertexBuffer(vertexBuffer, offset: 0, index: 0)
        // Similar to drawTriangles?
        renderCommandEncoder?.setFragmentBuffer(uniformBuffer, offset: 0, index: 3)
        renderCommandEncoder?.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: vertexArray.count)
        
        
        renderCommandEncoder?.endEncoding()
        commandBuffer?.present(drawable)
        commandBuffer?.commit()
        
        
    }
}
