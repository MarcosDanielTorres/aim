#include "PhysicsSystem.h"
#include "core/logger/logger.h"


PhysicsSystem::PhysicsSystem() {


	// Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
	// It is not directly used in this example but still required.
	JPH::Factory::sInstance = new JPH::Factory();

	// Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
	// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
	// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
	JPH::RegisterTypes();

	// We need a temp allocator for temporary allocations during the physics update. We're
	// pre-allocating 10 MB to avoid having to do allocations during the physics update.
	// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
	// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
	// malloc / free.
	temp_allocator = new JPH::TempAllocatorImpl(1000 * 1024 * 1024);

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	job_system.Init(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

	// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const JPH::uint cMaxBodies = 1024;

	// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
	const JPH::uint cNumBodyMutexes = 0;

	// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
	// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
	// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const JPH::uint cMaxBodyPairs = 1024;

	// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
	// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
	const JPH::uint cMaxContactConstraints = 1024;


	// Now we can create the actual physics system.
	//JPH::PhysicsSystem physics_system;
	//physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

	this->inner_physics_system.Init(
		cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
		broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);


	inner_physics_system.SetBodyActivationListener(&body_activation_listener);
	inner_physics_system.SetContactListener(&contact_listener);

	// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	JPH::BodyInterface& body_interface = this->inner_physics_system.GetBodyInterface();
}

JPH::BodyInterface& PhysicsSystem::get_body_interface() {
	return this->inner_physics_system.GetBodyInterface();
}

void PhysicsSystem::set_debug_camera_pos(glm::vec3 pos) {
	debugRenderer.mCameraPos = JPH::Vec3(pos.x, pos.y, pos.z);
}


JPH::BodyID PhysicsSystem::create_body(const aim::Components::Transform3D transform, JPH::Ref<JPH::Shape> shape, bool is_static) {
	auto settings = JPH::BodyCreationSettings(
		shape,
		JPH::Vec3(transform.pos.x, transform.pos.y, transform.pos.z),
		JPH::Quat::sIdentity(),
		is_static ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
		is_static ? Layers::NON_MOVING : Layers::MOVING);

	auto& body_interface = this->get_body_interface();
	auto body_id = body_interface.CreateAndAddBody(
		settings, is_static ? JPH::EActivation::DontActivate : JPH::EActivation::Activate);
	return body_id;
}


void PhysicsSystem::update_physics(float dt) {
	static JPH::BodyIDVector active_body_ids;
	static JPH::BodyIDVector body_ids;
	active_body_ids.clear();
	body_ids.clear();

	this->inner_physics_system.GetActiveBodies(JPH::EBodyType::RigidBody, active_body_ids);
	this->inner_physics_system.GetBodies(body_ids);

	//debug
	auto& lock_interface = this->inner_physics_system.GetBodyLockInterfaceNoLock();
	for (const auto id : body_ids) {
		JPH::BodyLockRead lock(lock_interface, id);
		if (!lock.Succeeded()) {
			continue;
		}

		const auto& body = lock.GetBody();

		body.GetShape()->Draw(
			&this->debugRenderer,
			body.GetCenterOfMassTransform(),
			JPH::Vec3::sReplicate(1.0f),
			JPH::Color::sGetDistinctColor(body.GetID().GetIndex()),
			false,
			true);

	}

	//	sphere->GetShape()->Draw(
	//		&physics_system.debugRenderer,
	//		sphere->GetCenterOfMassTransform(),
	//		JPH::Vec3::sReplicate(1.0f),
	//		JPH::Color::sGetDistinctColor(sphere->GetID().GetIndex()),
	//		false,
	//		true);

	if (!active_body_ids.empty()) {

		// We simulate the physics world in discrete time steps. 60 Hz is a good rate to update the physics system.
		const float cDeltaTime = 1.0f / 60.0f;

		// Now we're ready to simulate the body, keep simulating until it goes to sleep
		static JPH::uint step = 0;
		// Next step

		// Output current position and velocity of the sphere
		//JPH::RVec3 position = body_interface.GetCenterOfMassPosition(sphere_id);
		//JPH::Vec3 velocity = body_interface.GetLinearVelocity(sphere_id);
		//std::cout << "Step " << step << ": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")" << std::endl;

		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
		const int cCollisionSteps = (int)std::ceil(cDeltaTime / dt);

		// Step the world
		this->inner_physics_system.Update(dt, cCollisionSteps, this->temp_allocator, &this->job_system);

	}
}
