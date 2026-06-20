import UIKit
import Metal

class ViewController: UIViewController {

    var metalLayer: CAMetalLayer!

    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Setup background
        view.backgroundColor = .black
        
        // Setup Metal Layer for MoltenVK Vulkan rendering
        metalLayer = CAMetalLayer()
        if let device = MTLCreateSystemDefaultDevice() {
            metalLayer.device = device
            metalLayer.pixelFormat = .bgra8Unorm
            metalLayer.framebufferOnly = true
            metalLayer.frame = view.layer.frame
            view.layer.addSublayer(metalLayer)
        } else {
            print("Error: Metal is not supported on this device.")
        }
        
        // TODO: Pass the metalLayer memory address to the C++ Vulkan/MoltenVK renderer
        // e.g. startVita3K(Unmanaged.passUnretained(metalLayer).toOpaque())
    }

    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        metalLayer?.frame = view.layer.frame
    }
}
