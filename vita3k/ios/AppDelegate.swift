import UIKit

// Bridge header exposes: vita3k_ios_start, vita3k_ios_pump_events, vita3k_ios_shutdown
// These are declared in main_ios.cpp with extern "C" linkage.

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    var window: UIWindow?
    private var displayLink: CADisplayLink?

    func application(
        _ application: UIApplication,
        didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?
    ) -> Bool {

        // Build the main window and root view controller
        window = UIWindow(frame: UIScreen.main.bounds)
        let rootVC = ViewController()
        window?.rootViewController = rootVC
        window?.makeKeyAndVisible()

        // Give UIKit one run-loop tick so the ViewController's viewDidLoad
        // finishes and the CAMetalLayer is properly set up before we start
        // the C++ engine.
        DispatchQueue.main.async { [weak self, weak rootVC] in
            guard let self = self, let vc = rootVC else { return }
            self.startEmulator(metalLayer: vc.metalLayer)
        }

        return true
    }

    // MARK: - Emulator lifecycle

    private func startEmulator(metalLayer: CAMetalLayer?) {
        guard let layer = metalLayer else {
            print("[VitaiOS] ERROR: metalLayer is nil — cannot start emulator")
            return
        }

        let layerPtr = Unmanaged.passUnretained(layer).toOpaque()

        // Pass nil as app_path to open the built-in game browser.
        // Pass a path string to auto-launch a specific title.
        let result = vita3k_ios_start(layerPtr, nil)
        guard result == 0 else {
            print("[VitaiOS] vita3k_ios_start failed with code \(result)")
            return
        }

        // Drive the SDL event loop at display refresh rate
        displayLink = CADisplayLink(target: self, selector: #selector(pumpEvents))
        displayLink?.add(to: .main, forMode: .common)
    }

    @objc private func pumpEvents() {
        vita3k_ios_pump_events()
    }

    func applicationWillTerminate(_ application: UIApplication) {
        displayLink?.invalidate()
        displayLink = nil
        vita3k_ios_shutdown()
    }
}
