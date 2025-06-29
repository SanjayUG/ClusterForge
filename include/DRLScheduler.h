#pragma once

#include <vector>
#include <memory>
#include <queue>
#include <unordered_map>
#include <Eigen/Dense>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace ClusterForge {

class Node;
class Task;
class Cluster;

struct DRLState {
    Eigen::VectorXd node_resources;      // Current resource usage of all nodes
    Eigen::VectorXd task_requirements;   // Resource requirements of pending task
    Eigen::VectorXd node_health;         // Health scores of nodes
    Eigen::VectorXd historical_performance; // Historical performance metrics
    double cluster_load;                 // Overall cluster load
    int num_pending_tasks;               // Number of pending tasks
    
    DRLState() : cluster_load(0.0), num_pending_tasks(0) {}
};

struct DRLAction {
    int target_node_id;
    double confidence;
    std::vector<double> feature_importance;
    
    DRLAction() : target_node_id(-1), confidence(0.0) {}
};

struct DRLExplanation {
    std::string decision_reason;
    std::vector<std::string> feature_explanations;
    std::vector<double> attention_weights;
    double confidence_score;
    std::string meta_learning_context;
    
    DRLExplanation() : confidence_score(0.0) {}
};

class NeuralNetwork {
private:
    std::vector<Eigen::MatrixXd> weights_;
    std::vector<Eigen::VectorXd> biases_;
    std::vector<Eigen::VectorXd> activations_;
    std::vector<Eigen::VectorXd> gradients_;
    
    double learning_rate_;
    int input_size_;
    int output_size_;
    std::vector<int> hidden_sizes_;

public:
    NeuralNetwork(int input_size, const std::vector<int>& hidden_sizes, int output_size, double lr = 0.001);
    
    Eigen::VectorXd forward(const Eigen::VectorXd& input);
    void backward(const Eigen::VectorXd& target);
    void updateWeights();
    void saveModel(const std::string& filename);
    void loadModel(const std::string& filename);
    
    // Attention mechanism
    Eigen::VectorXd computeAttention(const Eigen::MatrixXd& features);
    std::vector<double> getAttentionWeights() const;
};

class MetaLearner {
private:
    NeuralNetwork base_model_;
    NeuralNetwork meta_model_;
    std::vector<DRLState> task_history_;
    std::vector<DRLAction> action_history_;
    std::vector<double> reward_history_;
    
    double meta_learning_rate_;
    int adaptation_steps_;

public:
    MetaLearner(int state_size, int action_size, double meta_lr = 0.01);
    
    DRLAction adaptAndPredict(const DRLState& state, const std::vector<DRLState>& few_shot_examples);
    void updateMetaModel(const std::vector<DRLState>& states, const std::vector<DRLAction>& actions, 
                        const std::vector<double>& rewards);
    void saveMetaModel(const std::string& filename);
    void loadMetaModel(const std::string& filename);
};

class DRLScheduler {
private:
    std::shared_ptr<Cluster> cluster_;
    NeuralNetwork q_network_;
    NeuralNetwork target_network_;
    MetaLearner meta_learner_;
    
    std::queue<std::shared_ptr<Task>> pending_tasks_;
    std::unordered_map<int, DRLState> task_states_;
    std::unordered_map<int, DRLAction> task_actions_;
    std::unordered_map<int, DRLExplanation> task_explanations_;
    
    boost::asio::io_context io_context_;
    boost::thread scheduler_thread_;
    std::atomic<bool> is_running_;
    
    // Hyperparameters
    double epsilon_;
    double epsilon_decay_;
    double epsilon_min_;
    double gamma_;
    int batch_size_;
    int update_frequency_;
    int experience_buffer_size_;
    
    // Experience replay buffer
    std::vector<std::tuple<DRLState, DRLAction, double, DRLState>> experience_buffer_;
    
    // Performance tracking
    std::vector<double> training_losses_;
    std::vector<double> reward_history_;
    int total_decisions_;
    int correct_decisions_;

public:
    explicit DRLScheduler(std::shared_ptr<Cluster> cluster);
    ~DRLScheduler();
    
    // Core scheduling
    bool scheduleTask(std::shared_ptr<Task> task);
    int selectOptimalNode(const TaskRequirements& requirements);
    void updateScheduler();
    
    // DRL methods
    DRLState createState(const std::shared_ptr<Task>& task);
    DRLAction selectAction(const DRLState& state, bool use_exploration = true);
    double computeReward(const DRLState& state, const DRLAction& action, const DRLState& next_state);
    void trainModel();
    void updateTargetNetwork();
    
    // Meta-learning
    void adaptToNewCluster(const std::vector<DRLState>& cluster_examples);
    DRLAction metaLearningPredict(const DRLState& state);
    
    // Explanation generation
    DRLExplanation generateExplanation(const DRLState& state, const DRLAction& action);
    std::string explainDecision(const std::shared_ptr<Task>& task, int selected_node);
    std::vector<std::string> getFeatureImportance(const DRLState& state);
    
    // Model management
    void saveModel(const std::string& filename);
    void loadModel(const std::string& filename);
    void resetModel();
    
    // Performance monitoring
    double getAccuracy() const;
    double getAverageReward() const;
    std::vector<double> getTrainingLosses() const { return training_losses_; }
    std::vector<double> getRewardHistory() const { return reward_history_; }
    
    // Configuration
    void setEpsilon(double epsilon) { epsilon_ = epsilon; }
    void setLearningRate(double lr);
    void setBatchSize(int size) { batch_size_ = size; }
    
    // Utility
    void start();
    void stop();
    bool isRunning() const { return is_running_; }
    std::string getStatusReport() const;

private:
    void schedulerLoop();
    void processPendingTasks();
    void updateExperienceBuffer(const DRLState& state, const DRLAction& action, 
                               double reward, const DRLState& next_state);
    Eigen::VectorXd stateToVector(const DRLState& state);
    DRLState vectorToState(const Eigen::VectorXd& vector);
    void logDecision(const std::shared_ptr<Task>& task, int selected_node, 
                    const DRLExplanation& explanation);
};

} // namespace ClusterForge 